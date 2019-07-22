# UdpMidiTest
A test application for the Response MIDI Gateway.

The Response MIDI Gateway provides a simple UDP string-based interface for transmitting and receiving MIDI over the network. This application is designed to help test and troubleshoot UDP MIDI messaging.

## About this ETCLabs Project
UdpMidiTest is open-source software (developed by a combination of end users and ETC employees in their free time) designed to interact with Electronic Theatre Controls products. This is not official ETC software. For challenges using, integrating, compiling, or modifying this software, we encourage posting on the Issues page of this project. ETC Support is not familiar with this software and will not be able to assist if issues arise. (Hopefully issues won't happen, and you'll have a lot of fun with these tools and toys!)

We also welcome pull requests for bug fixes and feature additions.

# Download
[Download Now For Windows](https://github.com/ETCLabs/UDPMidiTest/releases)

# How to Use

![screenshot](https://raw.githubusercontent.com/ETCLabs/UDPMidiTest/master/doc/screenshot_main.png)

When you run the application, the first step is to select the network card (NIC) which is connected to the same network as the MIDI gateway.
Next, in order to send MIDI to the gateway (which will be then transmitted through the "Out" connection of the gateway as physical MIDI), enter the IP address of the gateway.
Also enter the port number the gateway is configured for (configure the gateway using ETC configuration software)

You can also select whether to play the recieved or transmitted MIDI locally on your PC, by selecting a synthesizer and checking "Play RX" or "Play TX"

Once you have selected these options, press Start to start receiving and transmitting MIDI.

The Transmit tab will show a list of messages transmitted, and the recieve tab will show any messages received.

To Transmit you can select `Notes` which will give you a keyboard to send MIDI notes; or `Show Control` to send MIDI Show Control messages.

For show control messages, you can either enter data in hexadecimal format (`Hex Bytes`), or if you select `Eos Cue Format` you can enter data in Eos cue style (e.g. 3/401 means cuelist 3, cue 401)
