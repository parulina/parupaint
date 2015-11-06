#include "parupaintWindow.h"
// Chat command related stuff

#include "net/parupaintClientInstance.h"
#include "parupaintKeys.h"
#include "overlay/parupaintChat.h"

#include <QString>
#include <QDebug>

void ParupaintWindow::ChatMessage(QString str)
{
	if(str[0] == '/'){
		QString cmd = str;
		QString params = "";
		if(str.indexOf(" ") != -1){
			cmd = str.section(" ", 0, 0);
			params = str.section(" ", 1);
		}
		cmd = cmd.mid(1);
		return Command(cmd, params);
	}
	client->SendChat(str);
}

void ParupaintWindow::Command(QString cmd, QString params)
{
	chat->AddMessage(">> " + cmd + " " + params);
	qDebug() << cmd << params;

	if(!params.isEmpty()){
		if(cmd == "load"){
			client->LoadCanvas(params);

		} else if(cmd == "save"){
			client->SaveCanvas(params);

		// record commands
		} else if(cmd == "play") {
			client->PlayRecord(params, false);

		} else if(cmd == "script") {
			client->PlayRecord(params, true);
		}
	}

	if(cmd == "key"){
		if(params.isEmpty()){
			QStringList list = key_shortcuts->GetKeys();
			list.sort();
			list += "<br/>Usage: /key name=shortcut";
			list += "Set keys. Dialog hotkeys requires restart.";

			return chat->AddMessage(list.join("<br/>"));
		}
		key_shortcuts->AddKey(params);
		key_shortcuts->Save();
	}
	else if(cmd == "load"){
		if(params.isEmpty()) return chat->AddMessage("Usage: /load file<br/>Load a file on the server.");
		client->LoadCanvas(params);
	}
	else if(cmd == "save"){
		if(params.isEmpty()) return chat->AddMessage("Usage: /save file<br/>Saves to a file on the server.");
		client->SaveCanvas(params);
	}
	else if(cmd == "play"){
		if(params.isEmpty()) return chat->AddMessage("Usage: /play file<br/>Plays a record on the server.");
		client->PlayRecord(params, false);
	}
	else if(cmd == "script"){
		if(params.isEmpty()) return chat->AddMessage("Usage: /script file<br/>Plays a script (might be laggy!).");
		client->PlayRecord(params, true);
	}
	else if(cmd == "connect"){
		if(params.isEmpty()) return chat->AddMessage("Usage: /connect hostname [port]<br/>Reconnect to a different server.");

		// FIXME use default port
		QString host = params.section(" ", 0, 0);
		int port = 0;
		if(params.contains(' ')) port = params.section(" ", 1).toInt();

		client->Connect(host + (port > 0 ? QString(" %1").arg(port) : ""));
	}
	else if(cmd == "disconnect"){
		client->Disconnect("command");
		if(params.isEmpty()){
			client->Connect("localhost");
		}
	}
	else if(cmd == "name"){
		if(params.isEmpty()) return chat->AddMessage("Usage: /name name <br/>Set your nickname.");
		if(params.length() > 24) return;
		// FIXME name packet, save to cfg file
		client->SetNickname(params);
	}
	else if(cmd == "clear"){
		chat->ClearMessages();
	}
	else if(cmd == "help"){
		QStringList list = {
			"Server commands: /load /save /play /script",
			"Client commands: /connect /disconnect /name /clear"
		};
		return chat->AddMessage(list.join("<br/>"));
	}
}
