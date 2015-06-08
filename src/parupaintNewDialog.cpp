
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
	ParupaintDialog(parent, "new...")
{
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
	connect(label, &QPushButton::pressed, 
			[=]{
				auto hh = height->currentText();
				height->setEditText(width->currentText());
				width->setEditText(hh);
			});

	auto * wcont = new QWidget;
	auto * wlay = new QHBoxLayout;
	wlay->addWidget(width);
	wlay->addWidget(label);
	wlay->setAlignment(label, Qt::AlignCenter);
	wlay->addWidget(height);

	wcont->setFocusProxy(width);
	wcont->setLayout(wlay);

	wcont->setTabOrder(width, height);

	auto * enter = new QPushButton("create new");
	enter->setDefault(true);
	connect(enter, &QPushButton::pressed, [=]{
		int w = width->currentText().toInt();
		if(w <= 0) return width->setFocus();

		int h = height->currentText().toInt();
		if(h <= 0) return height->setFocus();

		emit NewSignal(w, h);		
	});

	this->layout()->addWidget(preview);
	this->layout()->addWidget(wcont);
	this->layout()->addWidget(enter);

	QSettings cfg;
	width->setEditText(cfg.value("canvas/lastwidth").toString());
	height->setEditText(cfg.value("canvas/lastheight").toString());

	this->setFocusProxy(wcont);
	this->setFocus();
}