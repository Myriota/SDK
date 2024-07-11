#!/usr/bin/expect -f

# host simulator test
if { $argc == 2 } {
  set command [lindex $argv 0]
  set log [lindex $argv 1]
  log_file $log
  proc send_string { string } { send $string }
  spawn sh -c "STARTTIME=1672531200 LATITUDE=-34.9251188 LONGITUDE=138.600888 $command"
}

# real hardware test
if { $argc == 1 } {
  set port [lindex $argv 0]
  proc send_string { string } {
    puts "\r\n$string"
    send $string
  }
  spawn sh -c "stty -F $port 9600 raw -echo"
  spawn -open [open $port w+]
}

proc abort { } { send_user "Timeout!\r\n" }
expect_before timeout abort

# wait a bit longer for the first returns
set timeout 20
expect "+STATE=INITIALIZING"
expect "+STATE=READY"
set timeout 3

## QUERIES
# comm check
send_string "AT\r"
expect "OK"
# message queue
send_string "AT+MSGQ=?\r"
expect -re {OK\+MSGQ=[0-9]{1,3}\s+}
# multiple commands
send_string "AT+MSGQ=?\r\nAT\r\n"
expect -re {OK\+MSGQ=[0-9]{1,3}\s+}
expect "OK"
# state
send_string "AT+STATE=?\t\r"
expect "OK+STATE=READY"
# sdk version
send_string "AT+VSDK=? \r"
expect -re {OK\+VSDK=[0-9]\.[0-9]\.[0-9]\s+}
# module id
send_string "AT+MID=?\r"
expect -re {OK\+MID=00[0-9a-f]{8} M[1-2]-2[3-4]\s+}
# regcode
send_string "AT+REGCODE=?\r"
expect -re {OK\+REGCODE=[0-9a-z]{25}\s+}
# time
send_string "AT+TIME=?\r"
expect -re {OK\+TIME=}
# location
send_string "AT+LOCATION=?\r"
expect -re {OK\+LOCATION=}
# no query command match - 1
send_string "AT+REGCOD=?\r"
expect "ERROR=UNKNOWN_QUERY_CMD"
# no query command match - 2
send_string "AT+SAVEMSG=?\r"
expect "ERROR=UNKNOWN_QUERY_CMD"
# no query command match - 3
send_string "AT+COmM=?\r"
expect "ERROR=UNKNOWN_QUERY_CMD"

## CONTROLS
# save message
send_string "AT+SAVEMSG\r"
expect "OK+SAVEMSG"
# save message with parameter
send_string "AT+SAVEMSG=1\r"
expect "FAIL+SAVEMSG"
# tx start
send_string "AT+TXSTART\r"
expect "FAIL+TXSTART"
# tx start with parameter - 1
send_string "AT+TXSTART=161000000,0,0,60\r"
expect "OK+TXSTART=161000000,0,0,60"
# tx start with parameter - 2
send_string "AT+TXSTART=161000000,1,1,60\r"
expect "OK+TXSTART=161000000,1,1,60"
# tx start with parameter - 3
send_string "AT+TXSTART=180000000,1,1,60\r"
expect "FAIL+TXSTART=180000000,1,1,60"
# tx start with parameter - 4
send_string "AT+TXSTART=140000000,1,1,60\r"
expect "FAIL+TXSTART=140000000,1,1,60"
# tx start with parameter - 5
send_string "AT+TXSTART=i61000000,1,1,60\r"
expect "FAIL+TXSTART=i61000000,1,1,60"
# tx start with parameter - 6
send_string "AT+TXSTART=161000000,0,0,1000\r"
expect "FAIL+TXSTART=161000000,0,0,1000"
# tx start with parameter - 7
send_string "AT+TXSTART=161000000\r"
expect "FAIL+TXSTART=161000000"
# tx start with parameter - 8
send_string "AT+TXSTART=161000000,1\r"
expect "FAIL+TXSTART=161000000,1"
# tx start with parameter - 9
send_string "AT+TXSTART=161000000,1,1\r"
expect "FAIL+TXSTART=161000000,1,1"
# tx stop
send_string "AT+TXSTOP\r"
expect "OK+TXSTOP"
# tx stop with parameter
send_string "AT+TXSTOP=1\r"
expect "FAIL+TXSTOP"
# gnss fix
send_string "AT+GNSSFIX\r"
expect "OK+GNSSFIX"
expect "OK+GNSSFIX"
# gnss fix with parameter
send_string "AT+GNSSFIX=1\r"
expect "FAIL+GNSSFIX"
# rssi start
send_string "AT+RSSI\r"
expect "FAIL+RSSI"
# rssi start with frequency - 1
send_string "AT+RSSI=360000000\r"
expect -re {OK\+RSSI=\-[0-9]{2,3}\s+}
# rssi start with frequency - 2
send_string "AT+RSSI=340000000\r"
expect "FAIL+RSSI=340000000"
# rssi start with frequency - 3
send_string "AT+RSSI=430000000\r"
expect "FAIL+RSSI=430000000"
# set time to 0 and verify
send_string "AT+TIME=0\r"
expect -re {OK\+TIME=[0-9]}
send_string "AT+TIME=?\r"
expect -re {OK\+TIME=[0-9]}
# set time and verify
send_string "AT+TIME=1673395800\r"
expect -re {OK\+TIME=167339580[0-1]}
set timeout 60
send_string "AT+TIME=?\r"
expect -re {OK\+TIME=167339[1-9][0-9][0-9][0-9]}
# set time parameter too long
send_string "AT+TIME=12345678901\r"
expect "FAIL+TIME=12345678901"
# set time parameter with unwanted characters
send_string "AT+TIME=1234567890a\r"
expect "FAIL+TIME=1234567890a"
# set location and verify
send_string "AT+LOCATION=-349205499,1386086737\r"
expect "OK+LOCATION=-349205499,1386086737"
send_string "AT+LOCATION=?\r"
expect "OK+LOCATION=-349205499,1386086737"
# set location to 0 and verify
send_string "AT+LOCATION=0,0\r"
expect "OK+LOCATION=0,0"
send_string "AT+LOCATION=?\r"
expect "OK+LOCATION=0,0"
set timeout 3
# set location with unwanted characters
send_string "AT+LOCATION=a,b\r"
expect "FAIL+LOCATION=a,b"
# set location with no ',' separater
send_string "AT+LOCATION=-349205499.1386086737\r"
expect "FAIL+LOCATION=-349205499.1386086737"
# set location with lat out of range
send_string "AT+LOCATION=-900000001,1386086737\r"
expect "FAIL+LOCATION=-900000001,1386086737"
# set location with lon out of range
send_string "AT+LOCATION=-349205499,1800000001\r"
expect "FAIL+LOCATION=-349205499,1800000001"
# schedule message
send_string "AT+SMSG=0102030405060708091011121314151617181920\r"
expect "OK+SMSG=0102030405060708091011121314151617181920"
# schedule message over length
send_string "AT+SMSG=010203040506070809101112131415161718192021\r"
expect "ERROR=INVALID_COMMAND"
# schedule message invalid characters
send_string "AT+SMSG=in02030405060708091011121314151617181920\r"
expect "ERROR=INVALID_PARAMETER"
# schedule message character number is odd
send_string "AT+SMSG=010203040506070809101112131415161718192\r"
expect "ERROR=INVALID_PARAMETER"
# no control command match - 1
send_string "AT+MID\r"
expect "ERROR=UNKNOWN_CONTROL_CMD"
# no control command match - 2
send_string "AT+MID=1\r"
expect "ERROR=UNKNOWN_CONTROL_CMD"

## ERRORS
# case sensitivity - prefix
send_string "At+RSSI\r"
expect "ERROR=INVALID_COMMAND"
# case sensitivity - command
send_string "AT+rssi\r"
expect "ERROR=UNKNOWN_CONTROL_CMD"
# command too long
send_string "12345678901234567890123456789012345678901234567890123456789012345678901234567890\r"
expect "ERROR=INVALID_COMMAND"
# make sure valid commands can still be processed
send_string "AT\r"
expect "OK"
# command too short
send_string "A\r"
expect "ERROR=INVALID_COMMAND"
# no prefix found - 1
send_string "A+REGCODE=?\r"
expect "ERROR=INVALID_COMMAND"
# no prefix found - 2
send_string "T+REGCODE=?\r"
expect "ERROR=INVALID_COMMAND"
# no prefix found - 3
send_string "+REGCODE=?\r"
expect "ERROR=INVALID_COMMAND"
# command too long
send_string "AT+REGCODEREGC=?\r"
expect "ERROR=INVALID_COMMAND"
# parameter too long
send_string "AT+SMSG=12345678901234567890123456789012345678901\r"
expect "ERROR=INVALID_COMMAND"

send_user "Done\r\n"
close
