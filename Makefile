.PHONY: build upload monitor clean

build:
	arduino-cli compile \
		--fqbn esp32:esp32:esp32 \
		--library lua

upload:
	arduino-cli upload --fqbn esp32:esp32:esp32 --port /dev/ttyUSB0

monitor:
	arduino-cli monitor --port /dev/ttyUSB0 --config baudrate=115200

clean:
	rm -rf build