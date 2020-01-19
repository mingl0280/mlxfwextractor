
# Mellanox Firmware Extractor 

## pre-extract operation:

extract the two xz files from `/opt/tms/bin/fw-SX-rel-9_3_8170-FIT.mfa`

Main Header: `<4D 46 41 52 00 00 01 00 00 00 00 00 00 00 00 01 00 00 01 00 00 09 68>`

    First XZ Header: `<FD 37 7A 58 5A>`
        Content: Location, Name, Description of Firmwares
    First XZ End: `<00 00 00 00 01 59 5A>`

    Second XZ Header: `<FD 37 7A 58 5A>`
        Content: Not Necessary
    Second XZ End: `<00 00 00 00 01 59 5A> `

    Third XZ Header: `<FD 37 7A 58 5A>`
        Content: All Firmwares Combined into One File
            After unxz-ing the contents
            Headers (x96): `<4D 54 46 57 8C DF D0 00>`
    Third XZ End: `<00 00 00 00 01 59 5A>`

End of File: `<A3 2F D0 09>`

rename these two xz files as fwSect1.xz and fwSect3.xz,  then unxz them.

## extract

1. put the fwSect1 and fwSect3 in the same directory as the cpp file. 
2. compile the executable: 
   ```
   *nix:
       <g++-8|clang++> extractor.cpp -O2 -o extractor -std=c++17 -lstdc++fs
   Windows:
       Open the sln file and you know what to do.
   ```
4. run the executable.
5. Splited firmwares will be in the Firmwares/ directory named after there identification string.
