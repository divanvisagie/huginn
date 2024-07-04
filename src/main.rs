use rumqttc::{AsyncClient, Event, MqttOptions, QoS};
use serde::{Deserialize, Serialize};
use tokio::time::Duration;

#[derive(Serialize, Deserialize, Debug)]
struct Message {
    id: String,
    message: String,
    models: Vec<String>,
    strategy: String,
}

async fn handle_message(payload: &str) {
    match serde_json::from_str::<Message>(payload) {
        Ok(message) => {
            println!(
                "Parsed Message:\n  id: {}\n  message: {}\n  models: {:?}\n  strategy: {}",
                message.id, message.message, message.models, message.strategy
            );
        }
        Err(e) => {
            eprintln!("Failed to parse message: {}", e);
        }
    }
}

#[tokio::main]
async fn main() {
    // Set up the MQTT client
    let mut mqtt_options = MqttOptions::new("huginn", "localhost", 1883);
    mqtt_options.set_keep_alive(Duration::from_secs(5));

    let (client, mut eventloop) = AsyncClient::new(mqtt_options, 10);

    // Subscribe to the topic
    client.subscribe("huginn/in", QoS::AtMostOnce).await.unwrap();

    println!("Connected to the broker!");

    // Event loop to process incoming messages
    loop {
        match eventloop.poll().await {
            Ok(Event::Incoming(rumqttc::Packet::Publish(publish))) => {
                let payload = String::from_utf8(publish.payload.to_vec()).unwrap();
                println!("Received message: {} from topic: {}", payload, publish.topic);
                handle_message(&payload).await;
            }
            Ok(_) => {}
            Err(e) => {
                eprintln!("Error in the event loop: {}", e);
                break;
            }
        }
    }
}
