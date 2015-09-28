
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
		this->setObjectName("CanvasPreviewWidget");
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
				painter.drawText(5, 15, "* SCALED");

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
	ParupaintDialog(parent, "new canvas..."), cwidth(0), cheight(0)
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

	// create width, height, middle buttons
	width = new QComboBox;
	width->addItems(dim_list);
	width->setEditable(true);
	width->setCompleter(nullptr);
	connect(width, &QComboBox::editTextChanged,
	       [=](const QString & str){
			QSettings cfg;
			cfg.setValue("canvas/lastwidth", str.toInt());
			preview->setPreviewWidth(str.toInt());
		});
	height = new QComboBox;
	height->addItems(dim_list);
	height->setEditable(true);
	height->setCompleter(nullptr);
	connect(height, &QComboBox::editTextChanged,
	       [=](const QString & str){
			QSettings cfg;
			cfg.setValue("canvas/lastheight", str.toInt());
			preview->setPreviewHeight(str.toInt());
		});

	auto * flip_button = new QPushButton("flip");
	flip_button->setToolTip("flip the values.");
	connect(flip_button, &QPushButton::pressed, [=]{
		auto hh = height->currentText();
		height->setEditText(width->currentText());
		width->setEditText(hh);
	});
	auto * cres_button = new QPushButton("take");
	cres_button->setToolTip("take the current canvas dimensions.");
	connect(cres_button, &QPushButton::pressed, [this]{
		if(cwidth && cheight){
			this->width->setEditText(QString::number(cwidth));
			this->height->setEditText(QString::number(cheight));
		}
	});
	flip_button->setMaximumHeight(25);
	cres_button->setMaximumHeight(25);
	flip_button->setMaximumWidth(40);
	cres_button->setMaximumWidth(40);

	auto * res_layout = new QHBoxLayout;
	res_layout->setMargin(0);

	res_layout->addWidget(width);
	res_layout->addWidget(flip_button, Qt::AlignCenter);
	res_layout->addWidget(cres_button, Qt::AlignCenter);
	res_layout->addWidget(height);


	// create buttons
	auto * button_layout = new QHBoxLayout;
	button_layout->setMargin(0);

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

	button_layout->addWidget(enter);
	button_layout->addWidget(resize);
	button_layout->addWidget(reset);

	// vboxlayout is set in ParupaintDialog();
	auto * main_layout = ((ParupaintDialogLayout*)this->layout());
	main_layout->addWidget(preview);
	main_layout->addLayout(res_layout);
	main_layout->addLayout(button_layout);

	this->setTabOrder(width, height);

	QSettings cfg;
	width->setEditText(cfg.value("canvas/lastwidth").toString());
	height->setEditText(cfg.value("canvas/lastheight").toString());

	this->setFocus();
}

void ParupaintNewDialog::setOriginalDimensions(int w, int h)
{
	cwidth = w;
	cheight = h;
}
