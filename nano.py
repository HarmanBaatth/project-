from flask import Flask
import paho.mqtt.client as mqtt

app = Flask(__name__)

# Store bin status and coordinates
bin_status = {}
bin_coords = {}

# MQTT connection handler
def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker")
    client.subscribe("bin/status")

# MQTT message handler
def on_message(client, userdata, msg):
    payload = msg.payload.decode()
    print(f"Received MQTT message: {payload}")
    parts = payload.split(":")
    if len(parts) == 3:
        bin_id, status, coords = parts
        bin_status[bin_id] = status
        bin_coords[bin_id] = coords

# MQTT client setup
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("broker.hivemq.com", 1883, 60)
client.loop_start()

# Flask route for dashboard
@app.route("/")
def dashboard():
    html = "<h1>Smart Bin Dashboard</h1>"
    html += "<table border=1><tr><th>Bin</th><th>Status</th></tr>"

    for i in range(1, 51):  # Bin1 to Bin50
        bin_id = f"Bin{i}"
        status = bin_status.get(bin_id, "EMPTY")
        color = "green" if status == "EMPTY" else "red"
        html += f"<tr><td>{bin_id}</td><td style='background:{color};color:white'>{status}</td></tr>"

    html += "</table>"

    # Generate route for FULL bins
    full_bins = [bin_coords[bin_id] for bin_id in bin_status if bin_status[bin_id] == "FULL"]
    if full_bins:
        waypoints = "/".join(full_bins)
        route_url = f"https://www.google.com/maps/dir/{waypoints}"
        html += f"<p><a href='{route_url}' target='_blank'>Generate Route</a></p>"

    return html

# Run the Flask app
if __name__ == "__main__":
    app.run(host="0.0.0.0", port=80)