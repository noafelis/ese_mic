# ese_mic

### Done:
- read in Mic values via ADC on Tiva C
- send values to RPI server via UDP over network
  - establish ports
  - transfer ADC task values via Mailbox to UDP task
  - send values to RPI server via UDP
- display received values on RPI server (via commandline)

### ToDos:
- convert ADC values to useful data (eg dB)
- have UDP send buffer recursively, in case message was bigger than what can be sent in one packet (1 byte)
  - dito for Mailbox buffer

### Nice to Have:
- make it so 1st button press starts, 2nd press stops measurement; data is sent continuously while measuring

### Would've been nice to have:
- nice GUI on rpi server, display values as graph
