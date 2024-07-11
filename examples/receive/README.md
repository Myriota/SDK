# Receive Message Example

The `OnReceiveMessage` event and `ReceiveMessage` function can be used to receive messages sent to your module. The [receive example](main.c) demonstrates the use of these functions.

The example implements two jobs `ReceiveJob` and `TransmitJob` that execute at their own cadence. The `TransmitJob` uses the `ScheduleMessage` function to send the following information via the Myriota network:

1. time when the message was scheduled for transmission
2. number of messages received by the device to date
3. time when the most recent message was received
4. first 10 bytes of the most recent receive message

Messages are scheduled periodically every 8 hours, i.e. 3 messages per day.

The `ReceiveJob` schedules additional messages with the same format but only executes when a new message is received by the device. This is achieved with the following line in `AppInit`:
```c
  ScheduleJob(ReceiveJob, OnReceiveMessage());
```
The `ReceiveJob` wakes when an `OnReceiveMessage` event is triggered by an arriving message. The message is recovered using the `ReceiveMessage` function and printed as a hexadecimal string. The example then schedules a message for transmission as described above and includes the first 10 bytes of the newly received message and the time of reception.

To test this receive example first build and program the example into your module with
```sh
cd examples/receive
make
updater.py -u receive.bin -p /dev/ttyUSB0 -s
```
then put your module outside with a clear view of the sky.

Messages can be sent to your Myriota module from [devicemanager.myriota.com](https://devicemanager.myriota.com). It typically takes around a day for a message to be received by your module. Please refer to the SLA for latency details. If running the [receive example](main.c) above the message will be printed on the debug port when it is received. The above example also schedules the first 10 bytes of the received message for transmission. These messages can be retrieved later using
```sh
message_store.py query <ID> > messages.json
```
where `<ID>` is the module id of your device. Sample content of `messages.json` looks like so:
```json
[
  {
    "Timestamp": 1689645509240,
    "Value": "b9f1b5640000cccccccccccccccccccccccccccc"
  },
  {
    "Timestamp": 1689684500997,
    "Value": "018ab6640100018ab66448656c6c6f2c20456172"
  }
]
```
The retrieved messages can then be [unpacked](unpack.py) with
```sh
cat messages.json | grep Value | awk '{print $2}' | tr -d '"' | unpack.py
```
to produce
```json
[
  {
    "Timestamp": 1689645497,
    "CountReceive": 0,
    "TimestampReceive": null,
    "MessageReceive": null
  },
  {
    "Timestamp": 1689684481,
    "CountReceive": 1,
    "TimestampReceive": 1689684481,
    "MessageReceive": "48656c6c6f2c20456172"
  }
]
```
The first message was scheduled before the device received a message as indicated by the `CountReceive` field being zero. Consequently, no receive time and message content are available (value `null`). The second record contains valid values for all fields. In this example the message `Hello, Earth!` was sent to the device. The hexadecimal message `48656c6c6f2c20456172` unpacks to `Hello, Ear`, i.e. the first 10 bytes of `Hello, Earth!`.

Note that if you send a message that is shorter than 10 bytes to your device, it will be padded with a `cc` pattern. For example, sending `Hello` to the device results in the unpacked message
```json
    "MessageReceive": "48656c6c6fcccccccccc"
```
