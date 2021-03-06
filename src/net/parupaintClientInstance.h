#ifndef PARUPAINTCLIENTINSTANCE_H
#define PARUPAINTCLIENTINSTANCE_H

#include <QHash>
#include <QJsonObject>

#include "parupaintClient.h"
#include "../core/parupaintBrush.h"

class ParupaintVisualCursor;
class ParupaintCanvasScene;
class ParupaintBrush;

class ParupaintClientInstance : public ParupaintClient
{
Q_OBJECT
	QString client_name;
	bool read_only, client_joined;
	int me;
	bool remote_password;

	// TODO rename pool -> scene
	ParupaintCanvasScene * pool;

	QHash<quint32, ParupaintVisualCursor*> brushes;
	ParupaintBrush shadow_brush;

	signals:
	void onSpectateChange(bool);
	void onChatMessage(const QString & msg, const QString & usr);
	void onConnect();
	void onDisconnect(const QString & reason = "");

	void onPlayerCountChange(int num);
	void onSpectatorCountChange(int num);

	public:
	ParupaintClientInstance(ParupaintCanvasScene*, QObject * = nullptr);
	virtual void send(const QString , const QJsonObject & = QJsonObject());
	void message(const QString &, const QByteArray &);

	void setName(const QString &);
	void setReadOnly(bool r);
	const QString name();
	bool readOnly();
	bool isJoined();

	bool remoteHasPassword();

	void doJoin(const QString & password = QString());
	void doLeave();
	void doName();
	void doReloadCanvas();
	void doReloadImage(int l = -1, int f = -1);
	void doChat(const QString & str = QString());
	void doTyping();
	void doBrushUpdate(ParupaintBrush * brush);

	void doLayerFrameAttribute(int l, int f, const QString & attr, const QJsonValue & val);
	void doLayerFrameChange(int l, int f, int lc, int fc, bool ext = false);
	void doPasteImage(int l, int f, int x, int y, const QImage &);
	void doFill(int l, int f, const QString &);
	void doNew(int w, int h, bool resize = false);
	void doInfo(const QString & attr, const QVariant & val);
	
	void doLayerVisibility(int l, bool visible);
	void doLayerName(int l, const QString & name);
	void doLayerMode(int l, int mode);
	void doProjectName(const QString & name);
	void doProjectFramerate(const qreal framerate);
	void doProjectBackgroundColor(const QColor & color);

	void doLoadLocal(const QString & filename);
	void doLoad(const QString & filename);
	void doSave(const QString & filename);
	void doPlay(const QString & filename, int limit = -1);
};

#endif
