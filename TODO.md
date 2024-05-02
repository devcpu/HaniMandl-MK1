# HaniMandl-MK1 TODOs:

- [ ] WiFi-Manager
- [ ] EEPORM
- [ ] Touch-Sensor
- [ ] Display
- [X] Scale
- [X] Servo
- [ ] Documentation
- [ ] I18N

# Buttons on scale:
- [ ] Start
- [ ] Fine
- [ ] Stop
- [ ] Hand | Auto (toggle)

# LEDs:

- [ ] on | dual error
- [ ] filling | dual ready
- [ ] auto | dual hand

# Fields in EEPROM

- [ ] Los-Number
- [ ] Date
- [ ] weight
- [ ] count


# Params

## run

- [x] los-number
- [x] date if Wifi in AP mode
- [x] weight_filling
- [ ] angle_max
- [ ] angle_fine
- [ ] weight_fine
- [ ] overfill
- [x] glass_weight

## Setup

- [ ] Beekeepers name
- [ ] angle_min_hard
- [ ] angle_max_hard
- [ ] angle_max
- [ ] angle_min

- [ ] angle_fine
- [ ] calibrate_weight
- [ ] glass_tolerance



## Request routes GET

### filling
### SetupWlan
### UpdateFirmware
### systeminfo
### reboot

### setup
* beekeeping
* angle_max_hard
* angle_min_hard
* angle_max
* angle_min
* angle_fine
* glass_tolerance

### calibrate


### setupfilling
* weight_filling
* weight_empty
* los_number
* date_filling

## Request routes POST

### start
### stop
### hand
### auto

### amg
### aml
### afg
### afl
### wfg
### wfl
