#include "parupaintAVWriter.h"

#include <QImage>
#include <QDebug>
#include <QSettings>
#include <QFileInfo>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/pixfmt.h>
}

#define errorCancel(err) { setError(err); return; }
#define ppavfunc(l, f) auto pp_##f = decltype(&f)( l.resolve(#f)); if(!pp_##f) errorCancel("Function pp_" #f " in " #l " could not be loaded.")

ParupaintAVWriter::~ParupaintAVWriter()
{
	ppavfunc(avutil, av_free);
	ppavfunc(avformat, av_write_trailer);
	ppavfunc(avcodec, avcodec_close);
	ppavfunc(avformat, avio_close);
	ppavfunc(avformat, avformat_free_context);

	if(frame){
		pp_av_free(frame);
	}
	if(format_context){
		if(!hasError()) pp_av_write_trailer(format_context);

		if(video_stream) pp_avcodec_close(video_stream->codec);

		if(format && !(format->flags & AVFMT_NOFILE)) pp_avio_close(format_context->pb);
		pp_avformat_free_context(format_context);
	}
}


ParupaintAVWriter::ParupaintAVWriter(QString filename, int w, int h, int fps) : error(),
	avformat("avformat"), avutil("avutil"), avcodec("avcodec"), swscale("swscale"),
	video_codec(nullptr), codec_context(nullptr), format_context(nullptr), format(nullptr), video_stream(nullptr), frame(nullptr), sws_context(nullptr)
{
#ifdef Q_OS_WIN
	qDebug() << "Checking windows dlls...";
	QDir appdir(QCoreApplication::applicationDirPath());
	appdir.setFilter(QDir::Files | QDir::NoSymLinks);
	foreach(QFileInfo file, appdir.entryInfoList()){
		QString filename = file.fileName();
		qDebug() << filename;
		if(filename.contains(QRegularExpression("^avcodec.*.dll"))) avcodec.setFileName(filename);
		if(filename.contains(QRegularExpression("^avutil.*.dll"))) avutil.setFileName(filename);
		if(filename.contains(QRegularExpression("^avformat.*.dll"))) avformat.setFileName(filename);
		if(filename.contains(QRegularExpression("^swscale.*.dll"))) swscale.setFileName(filename);
	}
#endif

	QSettings cfg;
	if(cfg.contains("library/avformat")) 	avformat.setFileName(cfg.value("library/avformat").toString());
	if(cfg.contains("library/avutil")) 	avutil.setFileName(cfg.value("library/avutil").toString());
	if(cfg.contains("library/avcodec")) 	avcodec.setFileName(cfg.value("library/avcodec").toString());
	if(cfg.contains("library/swscale")) 	swscale.setFileName(cfg.value("library/swscale").toString());

	if(!swscale.load())	errorCancel("Could not load " + swscale.fileName());
	if(!avcodec.load())	errorCancel("Could not load " + avcodec.fileName());
	if(!avformat.load())	errorCancel("Could not load " + avformat.fileName());
	if(!avutil.load())	errorCancel("Could not load " + avutil.fileName());

	ppavfunc(avformat, av_register_all);
	ppavfunc(avutil, av_log_set_level);
	ppavfunc(avformat, avformat_alloc_output_context2);
	ppavfunc(avcodec, avcodec_find_encoder);
	ppavfunc(avutil, av_get_pix_fmt_name);
	ppavfunc(avformat, av_dump_format);
	ppavfunc(avformat, avformat_new_stream);
	ppavfunc(avcodec, avcodec_open2);
	ppavfunc(avutil, av_frame_alloc);
	ppavfunc(avutil, av_image_alloc);
	ppavfunc(avformat, avio_open);
	ppavfunc(avformat, avformat_write_header);

	// register and silence it
	pp_av_register_all();
	pp_av_log_set_level(cfg.value("library/av_loglevel", 0).toInt());
	error.clear();

	pp_avformat_alloc_output_context2(&format_context, nullptr, nullptr, filename.toStdString().c_str());
	if(filename.endsWith(".apng")) filename = filename.replace("apng", "png");
	if(!format_context) errorCancel("Couldn't determine format from extension.");

	// put guessed output format to format
	format = format_context->oformat;
	if(format->video_codec == AV_CODEC_ID_NONE) errorCancel("Format is not video.");

	AVPixelFormat pix_fmt = AV_PIX_FMT_NONE;
	video_codec = pp_avcodec_find_encoder(format->video_codec);
	if(!video_codec) errorCancel("Encoder not found.");

	// Iterate over available formats and try to pick a good one
	const AVPixelFormat * pf = video_codec->pix_fmts;
	QStringList fmt_list;
	while (pf != NULL && *pf != AV_PIX_FMT_NONE) {
		fmt_list << pp_av_get_pix_fmt_name(*pf);
		pix_fmt = *pf;
		++pf;
	}
	qDebug() << "Available formats:" << fmt_list;
	// Pick one.
	if(format->video_codec == AV_CODEC_ID_GIF) pix_fmt = AV_PIX_FMT_BGR8;
	else if(fmt_list.contains("rgba")) 	   pix_fmt = AV_PIX_FMT_RGBA;
	else if(fmt_list.contains("yuv420p")) 	   pix_fmt = AV_PIX_FMT_YUV420P;
	else if(pix_fmt == AV_PIX_FMT_NONE) 	   pix_fmt = (*video_codec->pix_fmts);
	else 					   pix_fmt = AV_PIX_FMT_RGBA;

	// New stream to write to...
	if(!(video_stream = pp_avformat_new_stream(format_context, video_codec))) errorCancel("Couldn't create stream.");

	codec_context = video_stream->codec;
	video_stream->id = format_context->nb_streams-1;

	codec_context->codec_id = format->video_codec;
	codec_context->codec_type = AVMEDIA_TYPE_VIDEO;
	codec_context->bit_rate = cfg.value("library/av_bitrate", 100000).toInt();

	codec_context->width = w;
	codec_context->height = h;

	codec_context->time_base.den = fps;
	codec_context->time_base.num = 1;
	codec_context->gop_size = 10;
	codec_context->pix_fmt = pix_fmt;

	if (format_context->oformat->flags & AVFMT_GLOBALHEADER) codec_context->flags |= CODEC_FLAG_GLOBAL_HEADER;
	// Now write the format stuff i guess
	pp_av_dump_format(format_context, 0, filename.toStdString().c_str(), 1);

	AVDictionary * opts = nullptr;
	if(pp_avcodec_open2(codec_context, video_codec, &opts) < 0) errorCancel("Couldn't open codec.");

	// Init new frame... width/height not set for some reason?
	frame = pp_av_frame_alloc();
	if(!frame) errorCancel("Couldn't create frame.");

	frame->width = codec_context->width;
	frame->height = codec_context->height;
	frame->format = codec_context->pix_fmt;

	if(pp_av_image_alloc(frame->data, frame->linesize, codec_context->width, codec_context->height, (AVPixelFormat)frame->format, 32) < 0){
		errorCancel("Couldn't create image.");
	}
	qDebug() << "Created" << pp_av_get_pix_fmt_name(codec_context->pix_fmt) << "image" << frame->width << frame->height;

	// Finally, open file and write the header to it.
	if (pp_avio_open(&format_context->pb, filename.toStdString().c_str(), AVIO_FLAG_WRITE) < 0) {
		errorCancel("Couldn't open file.");
	}
	pp_avformat_write_header(format_context, NULL);
}


void ParupaintAVWriter::addFrame(const QImage & img)
{
	ppavfunc(avutil, av_opt_set);
	ppavfunc(avutil, av_opt_get);

	ppavfunc(avcodec, av_init_packet);
	ppavfunc(avformat, av_interleaved_write_frame);
	ppavfunc(avcodec, avcodec_encode_video2);
	ppavfunc(swscale, sws_getCachedContext);
	ppavfunc(swscale, sws_scale);

	uint8_t *srcplanes[3] = {0};
	int srcstride[3] = {0};

	srcplanes[0] = (uint8_t*)img.bits();
	srcstride[0] = img.bytesPerLine();

	// FIXME the ffmpeg documentation is very confusing.
	// I don't know how to disable the dithering. fix it...
	pp_av_opt_set(sws_context, "sws_dither", "none", 0);
	sws_context = pp_sws_getCachedContext(sws_context, codec_context->width, codec_context->height, AVPixelFormat(AV_PIX_FMT_BGRA),
							codec_context->width, codec_context->height, AVPixelFormat(codec_context->pix_fmt),
							SWS_FAST_BILINEAR | SWS_PRINT_INFO, nullptr, nullptr, nullptr);
	pp_sws_scale(sws_context, srcplanes, srcstride, 0, codec_context->height, frame->data, frame->linesize);

	int ret = -1;
	if (format_context->oformat->flags & AVFMT_RAWPICTURE) {
		AVPacket pkt;
		pp_av_init_packet(&pkt);
		pkt.flags        |= AV_PKT_FLAG_KEY;
		pkt.stream_index  = video_stream->index;
		pkt.data          = frame->data[0];
		pkt.size          = sizeof(AVPicture);
		ret = pp_av_interleaved_write_frame(format_context, &pkt);
	} else {
		AVPacket pkt = AVPacket();
		int got_packet = 0;
		pp_av_init_packet(&pkt);

		ret = pp_avcodec_encode_video2(codec_context, &pkt, frame, &got_packet);

		if (ret < 0) errorCancel("Couldn't encode video frame");

		if (!ret && got_packet && pkt.size) {
			pkt.stream_index = video_stream->index;
			ret = pp_av_interleaved_write_frame(format_context, &pkt);
		} else {
			ret = 0;
		}
	}
}
