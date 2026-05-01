import paho.mqtt.client as mqtt

BROKER = "localhost"  # or your broker IP
PORT = 1883
TOPIC = "test/topic"


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected")
        client.subscribe(TOPIC)
    else:
        print(f"Connection failed: {rc}")


def on_message(client, userdata, msg):
    payload = msg.payload.decode()
    print(f"Received: {payload} on {msg.topic}")

    # ---- YOUR LOGIC HERE ----
    if payload == "ON":
        print("Do something")
    elif payload == "OFF":
        print("Do something else")


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(BROKER, PORT, 60)
client.loop_forever()
