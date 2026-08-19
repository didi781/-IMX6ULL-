#ifndef MQTT_WIDGET_H
#define MQTT_WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QMqttClient>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QHBoxLayout>
#include <QVBoxLayout>

#define SENSOR_BACK_PATH ":/images/sensor_back.jpg"

class MqttWidget : public QWidget
{
    Q_OBJECT

public:
    MqttWidget(QWidget *parent = nullptr);
    ~MqttWidget();

    void startConnection();
    void stopConnection();
    void setWidgetBackground();

protected:
    void paintEvent(QPaintEvent *event) override;

signals:
    void returnRequested();

private slots:
    void onConnected();
    void onDisconnected();
    void onMessageReceived(const QByteArray &message, const QString &topic);
    void publishTemperature();
    void onSpeedUp();
    void onSpeedDown();
    void onPublishSpeed();
    void onToggleLed();
    void onToggleStm32Led();

private:
    void setupUi();
    void controlLed(const QString &cmd);

    QMqttClient *m_client;
    QPixmap backgroundPixmap;
    QTimer *m_tempTimer;

    QPushButton *m_btnSpeedUp;
    QPushButton *m_btnSpeedDown;
    QLabel *m_lblSpeed;
    QPushButton *m_btnPublishSpeed;
    QPushButton *m_btnLedToggle;
    QPushButton *m_btnStm32LedToggle;
    QLineEdit *m_txtStm32Temp;

    int m_speed;
    bool m_ledState;
    bool m_stm32LedState;

    static constexpr int TEMP_PUBLISH_INTERVAL = 30000;
    static constexpr int SPEED_STEP = 10;
};

#endif // MQTT_WIDGET_H