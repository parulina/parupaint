#include "parupaintAVWriter.h"

#include <QImage>
#include <QDebug>

extern "C"{
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
#define errorCancel(err) { setError(err); return; }

ParupaintAVWriter::~ParupaintAVWriter()
{
	if(frame){
		av_free(frame);
	}
	if(format_context){
		av_write_trailer(format_context);

		if(video_stream) avcodec_close(video_stream->codec);

		if(format && !(format->flags & AVFMT_NOFILE)) avio_close(format_context->pb);
		avformat_free_context(format_context);
	}
}

ParupaintAVWriter::ParupaintAVWriter(QString filename, int w, int h, int fps) : error(),
	video_codec(nullptr), codec_context(nullptr), format_context(nullptr), format(nullptr), video_stream(nullptr), frame(nullptr), sws_context(nullptr)
{
	// register and silence it
	av_register_all();
	av_log_set_level(0);
	error.clear();

	avformat_alloc_output_context2(&format_context, nullptr, nullptr, filename.toStdString().c_str());
	if(filename.endsWith(".apng")) filename = filename.replace("apng", "png");
	if(!format_context) errorCancel("Couldn't determine format from extension.");

	// put guessed output format to format
	format = format_context->oformat;
	if(format->video_codec == AV_CODEC_ID_NONE) errorCancel("Format is not video.");

	AVPixelFormat pix_fmt = AV_PIX_FMT_NONE;
	AVCodecID video_id = format->video_codec;
	video_codec = avcodec_find_encoder(video_id);
	if(!video_codec) errorCancel("Encoder not found.");

	// Iterate over available formats and try to pick a good one
	const AVPixelFormat * pf = video_codec->pix_fmts;
	QStringList fmt_list;
	while (pf != NULL && *pf != AV_PIX_FMT_NONE) {
		fmt_list << av_get_pix_fmt_name(*pf);
		pix_fmt = *pf;
		++pf;
	}
	qDebug() << "Available formats:" << fmt_list;
	// Pick one.
	if(format->video_codec == AV_CODEC_ID_GIF) pix_fmt = AV_PIX_FMT_BGR8;
	else if(fmt_list.contains("rgba")) 	   pix_fmt = AV_PIX_FMT_RGBA;
	else if(fmt_list.contains("yuv420p")) 	   pix_fmt = AV_PIX_FMT_YUV420P;
	else if(pix_fmt == AV_PIX_FMT_NONE) 	   pix_fmt = (*video_codec->pix_fmts);

	// New stream to write to...
	if(!(video_stream = avformat_new_stream(format_context, video_codec))) errorCancel("Couldn't create stream.");

	codec_context = video_stream->codec;
	video_stream->id = format_context->nb_streams-1;

	codec_context->codec_id = format->video_codec;
	codec_context->codec_type = AVMEDIA_TYPE_VIDEO;
	codec_context->bit_rate = 1000000; // TODO variable?

	codec_context->width = w;
	codec_context->height = h;

	codec_context->time_base.den = fps;
	codec_context->time_base.num = 1;
	codec_context->gop_size = 10;
	codec_context->pix_fmt = pix_fmt;

	if (format_context->oformat->flags & AVFMT_GLOBALHEADER) codec_context->flags |= CODEC_FLAG_GLOBAL_HEADER;
	// Now write the format stuff i guess
	av_dump_format(format_context, 0, filename.toStdString().c_str(), 1);

	AVDictionary * opts = nullptr;
	if(avcodec_open2(codec_context, video_codec, &opts) < 0) errorCancel("Couldn't open codec.");

	// Init new frame... width/height not set for some reason?
	frame = av_frame_alloc();
	if(!frame) errorCancel("Couldn't create frame.");

	frame->width = codec_context->width;
	frame->height = codec_context->height;
	frame->format = codec_context->pix_fmt;

	if(av_image_alloc(frame->data, frame->linesize, codec_context->width, codec_context->height, (AVPixelFormat)frame->format, 32) < 0){
		errorCancel("Couldn't create image.");
	}
	qDebug() << "Created" << av_get_pix_fmt_name(codec_context->pix_fmt) << "image" << frame->width << frame->height;

	// Finally, open file and write the header to it.
	if (avio_open(&format_context->pb, filename.toStdString().c_str(), AVIO_FLAG_WRITE) < 0) {
		errorCancel("Couldn't open file.");
	}
	avformat_write_header(format_context, NULL);
}


void ParupaintAVWriter::addFrame(const QImage & img)
{
	uint8_t *srcplanes[3] = {0};
	int srcstride[3] = {0};

	srcplanes[0] = (uint8_t*)img.bits();
	srcstride[0] = img.bytesPerLine();

	sws_context = sws_getCachedContext(sws_context, codec_context->width, codec_context->height, AVPixelFormat(PIX_FMT_BGRA),
							codec_context->width, codec_context->height, AVPixelFormat(codec_context->pix_fmt),
							SWS_LANCZOS, nullptr, nullptr, nullptr);
	sws_scale(sws_context, srcplanes, srcstride, 0, codec_context->height, frame->data, frame->linesize);

	int ret = -1;
	if (format_context->oformat->flags & AVFMT_RAWPICTURE) {
		AVPacket pkt;
		av_init_packet(&pkt);
		pkt.flags        |= AV_PKT_FLAG_KEY;
		pkt.stream_index  = video_stream->index;
		pkt.data          = frame->data[0];
		pkt.size          = sizeof(AVPicture);
		ret = av_interleaved_write_frame(format_context, &pkt);
	} else {
		AVPacket pkt = AVPacket();
		int got_packet = 0;
		av_init_packet(&pkt);

		ret = avcodec_encode_video2(codec_context, &pkt, frame, &got_packet);

		char str[AV_ERROR_MAX_STRING_SIZE] = {0};
		if (ret < 0) errorCancel(QString("Couldn't encode video frame: %1").arg(av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, ret)));

		if (!ret && got_packet && pkt.size) {
			pkt.stream_index = video_stream->index;
			ret = av_interleaved_write_frame(format_context, &pkt);
		} else {
			ret = 0;
		}
	}
}
