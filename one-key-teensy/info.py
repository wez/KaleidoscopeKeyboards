keyboard(
    name='one-key-teensy',
    fqbn='teensy:avr:teensyLC:usb=serialhid,speed=48,opt=osstd,keys=en-us',
    vid='0xFEED',
    pid='0x6006',
    product='one-key-teensy',
    manufacturer='Wez Furlong',
    libs=[
	"https://github.com/keyboardio/Kaleidoscope",
	"https://github.com/keyboardio/KeyboardioHID",
    ]
)
