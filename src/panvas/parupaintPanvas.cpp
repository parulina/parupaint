
#include "parupaintPanvas.h"
#include "parupaintLayer.h"


PanvasProjectInformation::PanvasProjectInformation()
{
	Name = "Untitled";
}

ParupaintPanvas::ParupaintPanvas() : Info()
{

}

// Initialize with given layers and frames...
ParupaintPanvas::ParupaintPanvas(QSize dim, _lint l, _fint f)
{
	New(dim, l, f);
}

void ParupaintPanvas::New(QSize dim, _lint l, _fint f)
{
	Info.Dimensions = dim;
	Clear();
	
	SetLayers(l);
	foreach(auto l, Layers){
		l->New(dim);
		l->SetFrames(f);
	}
}

void ParupaintPanvas::Resize(QSize dim)
{
	foreach(auto l, Layers){
		l->Resize(dim);
	}
}

void ParupaintPanvas::Clear()
{
	foreach(auto l, Layers){
		delete l;
	}
	Layers.clear();
}

void ParupaintPanvas::SetLayers(_lint l, _fint f)
{
	auto diff = l - GetNumLayers();
	while(diff != 0){

		if(diff < 0) {
			if(Layers.isEmpty()) break;
			Layers.removeLast();
			diff ++;
		} else {
			Layers.append(new ParupaintLayer(Info.Dimensions, f));
			diff --;
		}
	}
}

void ParupaintPanvas::AddLayers(_lint l, _lint n)
{
	if(l > GetNumLayers()) l = GetNumLayers(); // or -1?
	else if(l < 0) l = 0;

	while(n > 0){
		Layers.insert(l, new ParupaintLayer(Info.Dimensions, 1));
		n--;
	}
}

void ParupaintPanvas::RemoveLayers(_lint l, _lint n)
{
	if(Layers.isEmpty()) return;

	if(l > GetNumLayers()) l = GetNumLayers(); // or -1...
	else if(l < 0) l = 0;

	while(n > 0){
		if(l > GetNumLayers()) break;
		delete Layers.at(l);
		Layers.removeAt(l);
		n--;
	}
}

ParupaintLayer* ParupaintPanvas::GetLayer(_lint l)
{
	return Layers.at(l);
}

_fint ParupaintPanvas::GetTotalFrames()
{
	auto fn = 0;
	foreach(auto l, Layers){
		auto n = l->GetNumFrames();
		if(n > fn) fn = n;
	}
	return fn;
}

_lint ParupaintPanvas::GetNumLayers()
{
	return Layers.length();
}

QList<QImage> ParupaintPanvas::GetImageFrames()
{
	return QList<QImage>();
}

int ParupaintPanvas::GetWidth() const
{
	return Info.Dimensions.width();
}
int ParupaintPanvas::GetHeight() const 
{
	return Info.Dimensions.height();
}
