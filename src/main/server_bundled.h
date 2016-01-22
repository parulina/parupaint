#ifndef SERVER_BUNDLED_H
#define SERVER_BUNDLED_H

#include "../net/parupaintServerInstance.h"

class ParupaintBundledServer : public ParupaintServerInstance
{
	protected:
	virtual void message(ParupaintConnection * con, const QString & id, const QByteArray & array);
	private slots:
	void onPlayerConnect(ParupaintConnection *);
	void onPlayerDisconnect(ParupaintConnection *);
	void onPlayerJoin(ParupaintConnection *);
	void onPlayerLeave(ParupaintConnection *);

	public:
	ParupaintBundledServer(quint16 port, QObject * = nullptr);
};

#endif
