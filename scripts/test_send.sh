#!/bin/bash
# Define the JSON message
json_message='{"id":"12345","message":"Hello, world!","models":["llama3"],"strategy":"external"}'

# Send the JSON message using mosquitto_pub
mosquitto_pub -h localhost -t huginn/in -m "$json_message"
