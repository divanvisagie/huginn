use std::ffi::{c_char, CStr};

use rumqttc::{AsyncClient, Event, MqttOptions, QoS};
use serde::{Deserialize, Serialize};
use tokio::time::Duration;

mod clients;


const IN_TOPIC: &str = "huginn/in";
const OUT_TOPIC: &str = "huginn/out";
const DEFER_TOPIC: &str = "huginn/defer";
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));
#[derive(Serialize, Deserialize, Debug)]
struct Message {
    id: String,
    timestamp: u64,
    message: String,
    models: Vec<String>,
    strategy: String,
}

enum Strategy {
    /// Defer processing of the message to an external machine
    /// and send a wake signal to that machine
    External,

    /// Use the current machine to run the model
    Internal,

    /// Prioritise the models in descending order as provided 
    /// preferring external processing but falling back to internal
    /// calls if the external machine is not up
    Fallback,
}
impl Strategy {
    fn from_str(strategy: &str) -> Self {
        match strategy {
            "external" => Strategy::External,
            "internal" => Strategy::Internal,
            _ => Strategy::Fallback,
        }
    }
}

async fn handle_message(payload: &str, client: &AsyncClient) {
    match serde_json::from_str::<Message>(payload) {
        Ok(message) => {
            println!(
                "Parsed Message:\n  id: {}\n  message: {}\n  models: {:?}\n  strategy: {}",
                message.id, message.message, message.models, message.strategy
            );
            match Strategy::from_str(&message.strategy) {
                Strategy::External => {

                    let mac = std::env::var("HUGINN_REMOTE_MAC").unwrap();
                    wake_on_lan(mac.as_str());

                    client
                        .publish(DEFER_TOPIC, QoS::AtMostOnce, false, payload)
                        .await
                        .unwrap();
                }
                Strategy::Internal => {
                    eprintln!("Internal strategy not implemented");
                }
                Strategy::Fallback => {
                    eprintln!("Unknown strategy: {}", message.strategy);
                }
            }
        }
        Err(e) => {
            eprintln!("Failed to parse message: {}", e);
        }
    }
}

fn wake_on_lan(mac: &str) {
    unsafe {
        let mac_c = std::ffi::CString::new(mac).unwrap();
        send_wake_on_lan(mac_c.as_ptr());
    }
}

#[tokio::main]
async fn main() {


    unsafe {
        let btr: *const c_char = bridge_test();
        let greeting = CStr::from_ptr(btr).to_str().unwrap();
        println!("C says: {}", greeting);
    }

    let mut mqtt_options = MqttOptions::new("huginn", "localhost", 1883);
    mqtt_options.set_keep_alive(Duration::from_secs(5));

    let (client, mut eventloop) = AsyncClient::new(mqtt_options, 10);

    client
        .subscribe(IN_TOPIC, QoS::AtMostOnce)
        .await
        .unwrap();

    println!("Connected to the broker!");

    loop {
        match eventloop.poll().await {
            Ok(Event::Incoming(rumqttc::Packet::Publish(publish))) => {
                let payload = String::from_utf8(publish.payload.to_vec()).unwrap();
                println!(
                    "Received message: {} from topic: {}",
                    payload, publish.topic
                );
                handle_message(&payload, &client).await;
                // send a message to the out queue
            }
            Ok(_) => {}
            Err(e) => {
                eprintln!("Error in the event loop: {}", e);
                break;
            }
        }
    }
}
