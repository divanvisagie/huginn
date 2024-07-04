# huginn
A system service for language model management.

Huginn receives messages on an MQTT topic, processes them with a language model, and publishes the results to another MQTT topic.
It takes in a message and a message id and returns a message and a message id.

The id is treated as a string and is used to identify the message. The message is treated as a string and is used to generate a response.


