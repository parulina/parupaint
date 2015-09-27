
#include <QSize>

#include "parupaintLayer.h"
#include "parupaintFrame.h"

ParupaintLayer::~ParupaintLayer()
{
	Clear();
}

ParupaintLayer::ParupaintLayer()
{
	Clear();
}
ParupaintLayer::ParupaintLayer(QSize s, _fint n) : ParupaintLayer()
{
	New(s);
	SetFrames(n);
}
void ParupaintLayer::New(QSize s)
{
	Dimensions = s;	
}

void ParupaintLayer::Resize(QSize s)
{
	foreach(auto f, Frames){
		f->Resize(s);
	}
}

void ParupaintLayer::Clear()
{
	// Clear out extended frames first
	for(auto i = GetNumFrames()-1; i > 0; i--){
		if(!this->IsFrameReal(i)){
			Frames.removeAt(i);
		}
	}
	foreach(auto i, Frames){
		delete i;
	}
	Frames.clear();
}
void ParupaintLayer::Fill(_fint f, QColor col)
{
	auto * frame = this->GetFrame(f);
	if(frame){
		frame->ClearColor(col);
	}
}
void ParupaintLayer::Fill(QColor col)
{
	foreach(auto f, Frames){
		f->ClearColor(col);
	}
}


void ParupaintLayer::SetFrames(_fint f)
{
	auto diff = f - GetNumFrames();
	while(diff != 0){

		if(diff < 0) {
			if(Frames.isEmpty()) break;
			Frames.removeLast();
			diff ++;
		} else {
			Frames.append(new ParupaintFrame(Dimensions));
			diff --;
		}
	}
}

void ParupaintLayer::AddFrames(_fint f, _fint n)
{
	if(f > GetNumFrames()) f = GetNumFrames(); // or -1?
	else if(f < 0) f = 0;

	while(n > 0){
		Frames.insert(f, new ParupaintFrame(Dimensions));
		n--;
	}
}

void ParupaintLayer::RemoveFrames(_fint f, _fint n)
{
	if(Frames.isEmpty()) return;

	if(f > GetNumFrames()) f = GetNumFrames()-1;
	else if(f < 0) f = 0;

	while(n > 0){
		if(f > GetNumFrames()) break;
		delete Frames.at(f);
		Frames.removeAt(f);
		n--;
	}
}
void ParupaintLayer::ExtendFrame(_fint f, _fint n)
{
	if(Frames.isEmpty()) return;
	
	if(f > GetNumFrames()) f = GetNumFrames();
	else if(f < 0) f = 0;

	Frames.at(f)->SetExtended(true);
	while(n > 0){
		Frames.insert(f, Frames.at(f));
		n--;
	}
}
void ParupaintLayer::RedactFrame(_fint f, _fint n)
{
	if(Frames.isEmpty()) return;
	
	if(f > GetNumFrames()) f = GetNumFrames()-1;
	else if(f < 0) f = 0;
	
	if(!IsFrameExtended(f)) {
		return;
	}
	
	while(n > 0){
		n--;
		auto nf = f+1;
		if(nf < GetNumFrames()) {
			if(Frames.at(f) == Frames.at(nf)){
				Frames.removeAt(nf);
				continue;
			}
		}
		break;
	}
	if(!IsFrameExtended(f)){
		Frames.at(f)->SetExtended(false);
	}
}


ParupaintFrame * ParupaintLayer::GetFrame(_fint f) const
{
	if(Frames.isEmpty() || f >= Frames.size()) return nullptr;
	return Frames.at(f);
}

QChar ParupaintLayer::GetFrameChar(_fint f)
{
	switch(GetFrameExtendedDirection(f)){
		case FRAME_NOT_EXTENDED:
			return QChar('x');
		case FRAME_EXTENDED_LEFT:
			return QChar('<');
		case FRAME_EXTENDED_MIDDLE:
			return QChar('=');
		case FRAME_EXTENDED_RIGHT:
			return QChar('>');
		default:
			return QChar(' ');
	}
}

QString ParupaintLayer::GetFrameLabel(_fint f)
{
	if(IsFrameReal(f)) {
		QString str = "-0";
		auto d = GetFrameExtendedDirection(f);
		if(d == 2) {
			auto end = f;
			while(GetFrameExtendedDirection(end) 
				!= FRAME_EXTENDED_RIGHT){
				end++;
			}
			str = QString("-%2").arg(end-f);
		}
		auto fr = GetFrame(f);
		auto op = fr->GetOpacity();
		if(op != 1.0) {
			str = QString("%1-%2").arg(str).arg(int(op*255));
		}
		return str;
	}
	return QString("0-0");
}



bool ParupaintLayer::IsFrameExtended(_fint f)
{
	return (GetFrameExtendedDirection(f) != FRAME_NOT_EXTENDED);
}
bool ParupaintLayer::IsFrameReal(_fint f)
{
	auto ed = GetFrameExtendedDirection(f);
	return (ed == 0 || ed == 2);
}
bool ParupaintLayer::IsFrameValid(_fint f)
{
	if(Frames.isEmpty()) return false;
	
	if(f > GetNumFrames()) return false;
	if(f < 0) return false;
	
	return true;
}

// Best to check the ranges and such before using this.
FrameExtensionDirection ParupaintLayer::GetFrameExtendedDirection(_fint f)
{
	auto d = 0;
	if(IsFrameValid(f)) {
		if(f > 0 && Frames.at(f-1) == Frames.at(f)) {
			d++;
		}
		if(f < GetNumFrames()-1 && Frames.at(f+1) == Frames.at(f)) {
			d += 2;
		}
	}
	return FrameExtensionDirection(d);
}


_fint ParupaintLayer::GetNumFrames()
{
	return Frames.length();
}

_fint ParupaintLayer::GetNumRealFrames()
{
	auto tf = 0;
	ParupaintFrame * old_frame = nullptr;
	foreach(auto f, Frames){
		if(f != old_frame){
			tf ++;
			old_frame = f;
		}
	}
	return tf;
}
