// voiceSensing.js

const { Cheetah } = require("@picovoice/cheetah-node");
const { PvRecorder } = require("@picovoice/pvrecorder-node");

module.exports.initializeVoiceSensing = async (io, client, mqttControl) => {
  let cheetah;
  let isRecognizing = false;

  try {
    cheetah = new Cheetah("tUBd2EpT2eDgWSXvuMbLf+0/f/oGjm6TNmnC6c3sPqrCdnajnRX0dQ=="); // Replace with your Picovoice AccessKey
  } catch (err) {
    console.error(err);
    return;
  }

  const pvRecorder = new PvRecorder(cheetah.frameLength);
  pvRecorder.start();

  console.log(`Using device: ${pvRecorder.getSelectedDevice()}`);
  console.log("Listening...");

  io.on('connection', (socket) => {
    socket.on('start_sensing', () => {
      isRecognizing = true;
    });

    socket.on('stop_sensing', () => {
      isRecognizing = false;
    });
  });

  while (true) {
    const audioFrame = await pvRecorder.read();
    try {
      const [partialTranscript, isEndpoint] = cheetah.process(audioFrame);
      process.stdout.write(partialTranscript);

      if (isEndpoint === true) {
        const flushedTranscript = cheetah.flush();
        process.stdout.write(`${flushedTranscript}\n`);

        if (isRecognizing) {
          // Check if the flushedTranscript contains a recognized command
          if (flushedTranscript.includes("On")) {
            console.log("published ON . . . . . . .. ");
            // Publish "ON" command to MQTT
            client.publish(mqttControl, 'start_sensing');
          } else if (flushedTranscript.includes("Of")) {
            // Publish "OFF" command to MQTT
            console.log("published OFF . . . . . . .. ");
            client.publish(mqttControl, 'stop_sensing');
          }
        }
      }
    } catch (err) {
      console.error(err);
    }
  }
};
