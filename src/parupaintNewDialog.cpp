
#include <QComboBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPainter>
#include <QSettings>

#include <QDebug>

#include "parupaintNewDialog.h"

class ParupaintCanvasPreviewWidget : public QWidget
{
	QSize size;
	public:
	ParupaintCanvasPreviewWidget(QWidget * parent = nullptr):
		QWidget(parent)
	{
		size = QSize(128, 128);
		this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	}
	void setPreviewHeight(int height){
		size.setHeight(height);
		this->update();
	}
	void setPreviewWidth(int width){
		size.setWidth(width);
		this->update();
	}
	private:
	void paintEvent(QPaintEvent * event)
	{
		QPainter painter(this);
		painter.setBrush(Qt::gray);
		painter.fillRect(this->rect(), Qt::black);
		if(size.isValid()){
			QPoint center_point = this->rect().center();
			QPoint temp_point = center_point;
			QSize temp_size = size;

			temp_point.setX(center_point.x() - temp_size.width()/2);
			temp_point.setY(center_point.y() - temp_size.height()/2);

			QRect scale_rect = this->rect();
			scale_rect.adjust(5, 5, -5, -5);

			if(temp_size.width() > scale_rect.size().width() ||
				temp_size.height() > scale_rect.size().height()){

				painter.setCompositionMode(QPainter::CompositionMode_Difference);
				painter.drawText(0, 10, "* SCALED");

				if(temp_size.width() > scale_rect.size().width()){
					double factor = (double(scale_rect.size().width()) / double(temp_size.width()));
					temp_size.setWidth(temp_size.width() * factor);
					temp_size.setHeight(temp_size.height() * factor);
				}
				if(temp_size.height() > scale_rect.size().height()){
					double factor = (double(scale_rect.size().height()) / double(temp_size.height()));
					temp_size.setWidth(temp_size.width() * factor);
					temp_size.setHeight(temp_size.height() * factor);
				}
				temp_point.setX(center_point.x() - temp_size.width()/2);
				temp_point.setY(center_point.y() - temp_size.height()/2);
			}

			painter.drawRect(QRect(temp_point, temp_size));
		}
		return QWidget::paintEvent(event);
	}
};

ParupaintNewDialog::ParupaintNewDialog(QWidget * parent) : 
	ParupaintDialog(parent, "new..."), cwidth(0), cheight(0)
{

	this->SetSaveName("newDialog");
	this->setMinimumSize(300, 300);
	QStringList dim_list = {
		"120",
		"500",
		"540",
		"800",
		"1000",
		"1280",
		"1400",
		"1600",
	};

	auto * preview = new ParupaintCanvasPreviewWidget;

	width = new QComboBox;
	width->addItems(dim_list);
	width->setEditable(true);
	width->setCompleter(nullptr);
	height = new QComboBox;
	height->addItems(dim_list);
	height->setEditable(true);
	height->setCompleter(nullptr);

	connect(width, &QComboBox::editTextChanged,
	       [=](const QString & str){
			QSettings cfg;
			cfg.setValue("canvas/lastwidth", str.toInt());
			preview->setPreviewWidth(str.toInt());
		});
	connect(height, &QComboBox::editTextChanged,
	       [=](const QString & str){
			QSettings cfg;
			cfg.setValue("canvas/lastheight", str.toInt());
			preview->setPreviewHeight(str.toInt());
		});
	auto * label = new QPushButton("← x →");
	label->setToolTip("flip the values.");
	connect(label, &QPushButton::pressed, 
			[=]{
				auto hh = height->currentText();
				height->setEditText(width->currentText());
				width->setEditText(hh);
			});

	auto * wcont = new QWidget;
	auto * wlay = new QHBoxLayout;
	wlay->setMargin(0);

	wlay->addWidget(width);
	wlay->addWidget(label);
	wlay->setAlignment(label, Qt::AlignCenter);
	wlay->addWidget(height);

	wcont->setFocusProxy(width);
	wcont->setLayout(wlay);

	wcont->setTabOrder(width, height);

	auto * wbutton = new QWidget;
	auto * wbuttonlayout = new QHBoxLayout;
	wbuttonlayout->setMargin(0);

	auto * enter = new QPushButton("new");
	enter->setDefault(true);
	enter->setToolTip("create a new canvas with the given dimensions.");
	connect(enter, &QPushButton::pressed, [=]{
		int w = width->currentText().toInt();
		if(w <= 0) return width->setFocus();

		int h = height->currentText().toInt();
		if(h <= 0) return height->setFocus();

		emit NewSignal(w, h, false);
	});

	auto * resize = new QPushButton("resize");
	resize->setToolTip("resize the current canvas with the given dimensions.");
	connect(resize, &QPushButton::pressed, [=]{
		int w = width->currentText().toInt();
		if(w <= 0) return width->setFocus();

		int h = height->currentText().toInt();
		if(h <= 0) return height->setFocus();

		emit NewSignal(w, h, true);
	});

	auto * reset = new QPushButton("reset");
	reset->setToolTip("reset the current canvas, keeping its dimensions.");
	connect(reset, &QPushButton::pressed, [=]{
		if(!cwidth || !cheight) return;

		emit NewSignal(cwidth, cheight, false);
	});

	wbuttonlayout->addWidget(enter);
	wbuttonlayout->addWidget(resize);
	wbuttonlayout->addWidget(reset);

	wbutton->setLayout(wbuttonlayout);

	this->layout()->addWidget(preview);
	this->layout()->addWidget(wcont);
	this->layout()->addWidget(wbutton);

	QSettings cfg;
	width->setEditText(cfg.value("canvas/lastwidth").toString());
	height->setEditText(cfg.value("canvas/lastheight").toString());

	this->setFocusProxy(wcont);
	this->setFocus();
}

void ParupaintNewDialog::setOriginalDimensions(int w, int h)
{
	cwidth = w;
	cheight = h;
}
