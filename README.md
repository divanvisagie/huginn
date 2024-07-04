# huginn
A system service for language model management.

The goal of Huginn, is to be able to provide seemless language model interaction in a service that does not require the opening of another port on the machine, as well as the ability to select a more powerful machine to run a local model on or reach out to cloud models based on different strategies.

For example, I have a service that runs on a Raspberry PI that sometimes requires a call to an LLM, the PI does not have the resources to run a large model, however I have a desktop machine with an Nvidia GPU that is more than capable but is not always on, however it has wakeonlan enabled. 

Huggin, on receiving messages, should wake up the desktop machine using wakeonlan and then process the message on that machine by sending a request to the Ollama service running on that machine.

```mermaid
sequenceDiagram
    autonumber
    participant h as Huginn (pi)
    participant m as MQTT Shated Instance (pi)
    participant d as Huginn (desktop)
    participant l as LLM
    h->>m: Subscribe to topic input topic
    m->>h: Message Comes in
    h->>d: Send wakeonlan packet
    h->>m: Send message to external processing topic
    d->>d: Boots up with huginn in external processing mode
    d->>m: Subscribe to external processing topic
    m->>+d: Message comes in
    d->>+l: Send request to Ollama
    l-->>d: Response
    d->>-m: Output topic

```

## How
Huginn receives messages on an MQTT topic, processes them with a language model, and publishes the results to another MQTT topic.
It takes in a message and a message id and returns a message and a message id. The client will handle this accordingly.

The id is treated as a string in order to allow for the client to handle the message id as it sees fit, whether through compounding or other methods.

The messages are sent as messagepack messages in the following format:

```go
type Message struct {
    ID       string   `msgpack:"id"`
    Message  string   `msgpack:"message"`
    Models   []string `msgpack:"models"`
    Strategy string   `msgpack:"strategy"`
}
```

