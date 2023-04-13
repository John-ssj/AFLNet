mqttParser_objects = ../MQTTParser.o ../MQTTProperties.o ../MQTTHelper.o
mqttParser_obj: $(mqttParser_objects)

../MQTTParser.o : MQTTParser.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@
../MQTTProperties.o : MQTTProperties.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@
../MQTTHelper.o : MQTTHelper.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

.PHONY : clean

clean:
	rm $(mqttParser_objects)