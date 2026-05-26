import paho.mqtt.client as mqtt
from flask import Flask, render_template, redirect, url_for

app = Flask(__name__)

# Initialize MQTT Client
mqttc = mqtt.Client()
mqttc.connect("localhost", 1883, 60)
mqttc.loop_start()

# Track active attack states for both ESP32 units
devices_status = {
    "esp32_1": "None",
    "esp32_2": "None"
}

@app.route("/")
def main():
    return render_template('main.html', status=devices_status)

# --- ESP32 Unit 1 Routes (Topic: ESP32/freeze) ---
@app.route("/esp1/freeze")
def esp1_freeze():
    mqttc.publish("ESP32/freeze", "0")
    devices_status["esp32_1"] = "Freeze"
    return redirect(url_for('main'))

@app.route("/esp1/lightning")
def esp1_lightning():
    mqttc.publish("ESP32/freeze", "1")
    devices_status["esp32_1"] = "Lightning"
    return redirect(url_for('main'))


# --- ESP32 Unit 2 Routes (Topic: ESP32/zap) ---
@app.route("/esp2/freeze")
def esp2_freeze():
    # Sends 0 to the zap subtopic
    mqttc.publish("ESP32/zap", "0")
    devices_status["esp32_2"] = "Freeze"
    return redirect(url_for('main'))

@app.route("/esp2/lightning")
def esp2_lightning():
    # Sends 1 to the zap subtopic
    mqttc.publish("ESP32/zap", "1")
    devices_status["esp32_2"] = "Lightning"
    return redirect(url_for('main'))

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=8181, debug=True)