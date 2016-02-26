#include "parupaintProjectInfo.h"

#include "../core/parupaintPanvas.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QDebug>

#include "../widget/parupaintLineEdit.h"

ParupaintProjectInfo::ParupaintProjectInfo(QWidget * parent) : 
	QFrame(parent)
{
	this->setFocusPolicy(Qt::ClickFocus);
	this->setContentsMargins(0, 0, 0, 0);

	line_title = new QLabel("Blabla: 1440x1440, 2/30 frames");
	line_title->setObjectName("ProjectQuickInfo");

	line_name = new ParupaintLineEdit(nullptr, "Project name");
	line_fps = new ParupaintSpinBox(nullptr);
	line_bgc = new ParupaintLineEdit(nullptr, "#FFFFFFFF");

	line_name->setFocusPolicy(Qt::ClickFocus);
	line_fps->setFocusPolicy(Qt::ClickFocus);
	line_bgc->setFocusPolicy(Qt::ClickFocus);

	line_title->setFixedHeight(20);

	QFormLayout * form_layout = new QFormLayout;
		form_layout->setMargin(0);
		form_layout->addRow("Project name", line_name);
		form_layout->addRow("Frames/second", line_fps);
		form_layout->addRow("Background color", line_bgc);


	QVBoxLayout * layout = new QVBoxLayout;
		layout->addWidget(line_title, 0, Qt::AlignTop | Qt::AlignHCenter);
		layout->addSpacing(10);
		layout->addLayout(form_layout, 0);
		layout->addStretch(1);
	this->setLayout(layout);

	connect(line_name, &QLineEdit::textEdited, this, &ParupaintProjectInfo::projectNameChanged);
	connect(line_fps, static_cast<void(ParupaintSpinBox::*)(double)>(&ParupaintSpinBox::valueChanged),
		this, &ParupaintProjectInfo::frameRateChanged);
	// TODO proper color stuff
	connect(line_bgc, &QLineEdit::textEdited, [=](const QString & text){
			QColor color(text);
			if(color.isValid()){
				emit backgroundColorChanged(color);
			}
	});

	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

void ParupaintProjectInfo::updateCanvasSlot()
{
	ParupaintPanvas * canvas = qobject_cast<ParupaintPanvas*>(sender());
	if(canvas){
		QString t = QString("[%1]: %2x%3, %4 frames").
			arg(canvas->projectDisplayName()).
			arg(canvas->dimensions().width()).
			arg(canvas->dimensions().height()).
			arg(canvas->totalFrameCount());
		line_title->setText(t);

		line_name->setText(canvas->projectName());
		bool s = this->blockSignals(true);
			line_fps->setValue(canvas->frameRate());
		this->blockSignals(s);
		line_bgc->setText(canvas->backgroundColor().name(QColor::HexArgb));
	}
}

QSize ParupaintProjectInfo::minimumSizeHint() const
{
	return QSize(300, 20);
}
QSize ParupaintProjectInfo::sizeHint() const
{
	return QSize(this->minimumSizeHint().width(), 200);
}

