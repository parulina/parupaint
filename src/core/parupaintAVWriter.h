#ifndef PARUPAINTAVWRITER_H
#define PARUPAINTAVWRITER_H

#include <QtCore/QtCore>

class AVCodec;
class AVCodecContext;
class AVFormatContext;
class AVOutputFormat;
class AVStream;
class AVFrame;
struct SwsContext;

class ParupaintAVWriter
{
	private:
	QString error;
	AVCodec * video_codec;
	AVCodecContext * codec_context;

	AVFormatContext * format_context;
	AVOutputFormat * format;
	AVStream * video_stream;
	AVFrame *frame;
	struct SwsContext * sws_context;

	void setError(QString err = "") {error = err;}

	public:
	ParupaintAVWriter(QString filename, int w, int h, int fps=24);
	~ParupaintAVWriter();
	void addFrame(const QImage & img);

	bool hasError() {return !error.isEmpty(); }
	const QString & errorStr() { return error; }

};
#endif
