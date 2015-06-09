
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSettings>
#include <QDir>
#include <QFileInfo>

#include "parupaintConnection.h"
#include "parupaintServerInstance.h"
#include "../core/parupaintPanvasReader.h"
#include "../core/parupaintPanvasWriter.h"
#include "../core/parupaintPanvas.h"
#include "../core/parupaintLayer.h"
#include "../core/parupaintFrame.h"

#include "../core/parupaintBrush.h"

#include "qcompressor.h"

ParupaintServerInstance::ParupaintServerInstance(quint16 port, QObject * parent) : ParupaintServer(port, parent)
{
	connectid = 1;
	canvas = new ParupaintPanvas(540, 540, 1);
	
	
	canvas->GetLayer(0)->GetFrame(0)->ClearColor(Qt::white);
	connect(this, &ParupaintServer::onMessage, this, &ParupaintServerInstance::Message);
}

ParupaintPanvas * ParupaintServerInstance::GetCanvas()
{
	return canvas;
}

QString ParupaintServerInstance::MarshalCanvas()
{
	QJsonObject obj;
	obj["width"] = canvas->GetWidth();
	obj["height"] = canvas->GetHeight();

	QJsonArray layers;
	for(auto l = 0; l < canvas->GetNumLayers(); l++){
		auto * layer = canvas->GetLayer(l);
		if(!layer) continue;

		QJsonArray frames;
		for(auto f = 0; f < layer->GetNumFrames(); f++){
			auto * frame = layer->GetFrame(f);

			QJsonObject fobj;
			fobj["extended"] = !(layer->IsFrameReal(f));
			fobj["opacity"] = frame->GetOpacity();

			frames.insert(f, fobj);
		}
		layers.insert(l, frames);
	}
	obj["layers"] = layers;
		

	return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

int ParupaintServerInstance::GetNumConnections()
{
	return brushes.size();
}

void ParupaintServerInstance::Message(ParupaintConnection * c, const QString id, const QByteArray bytes)
{
	QJsonObject obj = QJsonDocument::fromJson(bytes).object();

	if(c) {
		if(id == "connect"){
		
		} else if(id == "disconnect") {
			if(c && c->id){
				delete brushes[c];
				brushes.remove(c);
				// remove the brush
				QJsonObject obj2;
				obj2["disconnect"] = true;
				obj2["id"] = c->id;
				this->Broadcast("peer", QJsonDocument(obj2).toJson(QJsonDocument::Compact));
			}

		} else if(id == "join"){
			c->id = (connectid++);

			auto name = obj["name"].toString();
			brushes[c] = new ParupaintBrush();
			brushes[c]->SetName(name);

			c->send(id, "{\"name\":\"sqnya\"}");
			c->send("canvas", MarshalCanvas());
			

			foreach(auto c2, brushes.keys()){
				auto * them = brushes.value(c2);

				QJsonObject obj2;
				obj2["disconnect"] = false;
				
				// send me to others
				obj2["name"] = brushes[c]->GetName();
				obj2["id"] = c->id;
				if(c2 == c) obj2["id"] = -(c->id);
				c2->send("peer", QJsonDocument(obj2).toJson(QJsonDocument::Compact));

				if(c2 == c) continue;

				// send them to me
				obj["name"] = them->GetName();
				obj["id"] = c2->id;
				c->send("peer", QJsonDocument(obj).toJson(QJsonDocument::Compact));
				
			}

		} else if(id == "lf") {

			int   layer = obj["l"].toInt(),
			      frame = obj["f"].toInt(),
			      layer_change = obj["ll"].toInt(),
			      frame_change = obj["ff"].toInt();
			bool  extended = obj["ext"].toBool();

			bool changed = false;
			if(layer_change != 0){
				if(layer_change < 0 && canvas->GetNumLayers() > 1){
					canvas->RemoveLayers(layer, -layer_change);
					changed = true;
				} else if(layer_change > 0){
					canvas->AddLayers(layer, layer_change, 1);
					changed = true;
				}
			}
			if(frame_change != 0){
				auto * ff = canvas->GetLayer(layer);
				if(ff) {
					if(frame_change < 0 && ff->GetNumFrames() > 1){
						if(extended){
							ff->RedactFrame(frame, -frame_change);
						} else {
							ff->RemoveFrames(frame, -frame_change);
						}
						changed = true;
					} else if(frame_change > 0){
						if(extended){
							ff->ExtendFrame(frame, frame_change);
						} else {
							ff->AddFrames(frame, frame_change);
						}
						changed = true;
					}
				}
			}
			if((layer_change != 0 || frame_change != 0) && changed){
				this->Broadcast("canvas", MarshalCanvas());
			}
			auto * brush = brushes.value(c);
			if(brush) {
				if(layer < 0) layer = int(canvas->GetNumLayers())-1;
				if(layer >= int(canvas->GetNumLayers())) layer = 0;
				auto * ll = canvas->GetLayer(layer);
				if(ll){
					if(frame < 0) frame = int(ll->GetNumFrames())-1;
					if(frame >= int(ll->GetNumFrames())) frame = 0;
				}
				brush->SetLayer(layer);
				brush->SetFrame(frame);
				obj["l"] = layer;
				obj["f"] = frame;
				obj["id"] = c->id;
				this->Broadcast(id, obj);

			}

		} else if(id == "draw"){
			auto * brush = brushes.value(c);
			if(brush) {

				QColor color = HexToColor(obj["c"].toString());
				auto drawing(obj["d"].toBool());
				auto width(obj["s"].toDouble());
				auto x(obj["x"].toDouble());
				auto y(obj["y"].toDouble());

				auto old_x = brush->GetPosition().x();
				auto old_y = brush->GetPosition().y();
				auto l = brush->GetLayer();
				auto f = brush->GetFrame();

				brush->SetColor(color);
				brush->SetWidth(width);

				if(drawing && !brush->IsDrawing()){
					x = old_x;
					y = old_y;
				}
				if(drawing){
					auto * layer = canvas->GetLayer(l);
					if(layer) {
						auto * frame = layer->GetFrame(f);
						if(frame){
							frame->DrawStep(old_x, old_y, x, y, width, color);
						}
					}
				}

				brush->SetPosition(QPointF(x, y));
				brush->SetDrawing(drawing);

				obj["id"] = c->id;
				this->Broadcast(id, obj);
			}
		} else if(id == "canvas") {
			c->send("canvas", this->MarshalCanvas());

		} else if (id == "img") {

			for(auto l = 0; l < canvas->GetNumLayers(); l++){
				auto * layer = canvas->GetLayer(l);
				for(auto f = 0; f < layer->GetNumFrames(); f++){
					auto * frame = layer->GetFrame(f);
					if(!layer->IsFrameReal(f)) continue;

					const auto img = frame->GetImage();
					
 					const QByteArray fdata((const char*)img.bits(), img.byteCount());
					QByteArray compressed_bytes;
					QCompressor::gzipCompress(fdata, compressed_bytes);

					QJsonObject obj;
					obj["data"] = QString(compressed_bytes.toBase64());
					obj["w"] = img.width();
					obj["h"] = img.height();
					obj["l"] = l;
					obj["f"] = f;
					c->send("img", QJsonDocument(obj).toJson(QJsonDocument::Compact));
				}
			}
		} else if(id == "save") {
			QSettings cfg;
			auto name = obj["filename"].toString();
			auto load_dir = cfg.value("canvas/directory").toString();

			if(name.section(".", 0, 0).isEmpty()){
				// just put it as today's date then.
				QDateTime time = QDateTime::currentDateTime();
				name = "drawing_at_"+ time.toString("yyyy-MM-dd_HH.mm") + name;
			}

			if(!name.isEmpty()){
				ParupaintPanvasWriter writer(canvas);
				// Loader handles name verification
				auto ret = writer.Save(load_dir, name);
				if(ret == PANVAS_WRITER_RESULT_OK){
					QString msg = QString("Server saved canvas successfully at: \"%1\"").arg(name);
					this->BroadcastChat(msg);
				}
				
			}
		} else if(id == "new") {
			if(obj["width"].isNull()) return;
			if(obj["height"].isNull()) return;

			int width = obj["width"].toInt();
			int height = obj["height"].toInt();

			if(width <= 0 || height <= 0) return;

			canvas->Clear();
			canvas->Resize(QSize(width, height));
			canvas->AddLayers(0, 1, 1);

			QString msg = QString("New canvas: %1 x %2").arg(width).arg(height);
			this->BroadcastChat(msg);

			this->Broadcast("canvas", MarshalCanvas());

		} else if(id == "load") {

			QSettings cfg;
			QString load_dir = cfg.value("server/directory").toString();

			if(obj["file"].isString() && obj["filename"].isString()){

				QByteArray data = QByteArray::fromBase64(obj["file"].toString().toUtf8());
				QByteArray uncompressed;
				QCompressor::gzipDecompress(data, uncompressed);

				QString temp_file = obj["filename"].toString();

				load_dir = QDir::temp().path();
				QFileInfo ff(load_dir, temp_file);

				QFile file(ff.filePath());
				if(!file.open(QIODevice::WriteOnly)){
					obj["filename"] = QJsonValue::Undefined;
					return;
				}
				file.write(uncompressed);
				file.close();
			}

			if(obj["filename"].isString()){
				auto name = obj["filename"].toString();

				if(name.isEmpty()) return;
				ParupaintPanvasReader reader(canvas);
				// Loader handles name verification
				// TODO name verification here pls
				auto ret = reader.Load(load_dir, name);
				if(ret == PANVAS_READER_RESULT_OK){
					qDebug() << "Loaded canvas fine.";
					QString msg = QString("Server loaded file successfully at: \"%1\"").arg(name);
					this->BroadcastChat(msg);

					this->Broadcast("canvas", MarshalCanvas());
				}
			}

		} else if(id == "chat") {
			auto msg = obj["message"].toString();
			if(msg.isEmpty()) msg = obj["msg"].toString();

			auto * brush = brushes.value(c);
			if(brush) {
				obj["name"] = brush->GetName();
				obj["id"] = c->id;
				this->Broadcast(id, obj);
			}
		} else {
			//qDebug() << id << obj;
		}
	}

}

void ParupaintServerInstance::BroadcastChat(QString str)
{
	QJsonObject obj;
	obj["message"] = str;
	this->Broadcast("chat", obj);
}

void ParupaintServerInstance::Broadcast(QString id, QJsonObject ba, ParupaintConnection * c)
{
	this->Broadcast(id, QJsonDocument(ba).toJson(QJsonDocument::Compact), c);
}
void ParupaintServerInstance::Broadcast(QString id, QString ba, ParupaintConnection * c)
{
	return this->Broadcast(id, ba.toUtf8(), c);
}
void ParupaintServerInstance::Broadcast(QString id, const QByteArray ba, ParupaintConnection * c)
{
	foreach(auto i, brushes.keys()){
		if(i != c){
			i->send(id, ba);
		}
	}
}

