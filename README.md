## Ninja Gaiden / Ninja Gaiden Black Encryption Tool

Encryption/decryption tool for Ninja Gaiden or Ninja Gaiden Black gamesave or settings data. Allows displaying or changing the current XUID associated with a given Ninja Gaiden gamesave. Automatically re-signs gamesave or settings data to a given XboxHDKey.

### Usage (command line):

##### Decrypt file
> ninjacrypt.exe decrypt [file]

##### Encrypt file
> ninjacrypt.exe encrypt [XboxHDKey] [file]

##### Show XUID (Ninja Gaiden only)
> ninjacrypt.exe xuid [file]

##### Change XUID (Ninja Gaiden only)
> ninjacrypt.exe xuid [XUID] [XboxHDKey] [file]

<br>

___NOTE: XUID functions are for Ninja Gaiden gamesave data only___

------

#### Gamesave location:

##### Ninja Gaiden
> /UDATA/54430003/[SaveID]/`saveXXX.dat`

> /TDATA/54430003/`system.dat`

##### Ninja Gaiden Black
> /UDATA/5443000D/[SaveID]/`saveXXX.dat`

> /TDATA/5443000D/`system.dat`

------

#### XboxHDKey (per-console key)

* [Retrieve the XboxHDKey from a **Xbox** ](https://github.com/feudalnate/Original-Xbox-Gamesave-Resigners/blob/master/XboxHDKey.md#retrieving-the-xboxhdkey-from-the-xbox)
* [Retrieve the XboxHDKey from a **Xbox 360** ](https://github.com/feudalnate/Original-Xbox-Gamesave-Resigners/blob/master/XboxHDKey.md#retrieving-the-xboxhdkey-from-the-xbox-360)

------

#### XUID (per-account unique user ID)

##### Xbox
* [Retrieve or generate account information using Xbox Account Manager](https://github.com/feudalnate/Original-Xbox-LIVE-Account)

##### Xbox 360

XUID can be recovered from an _existing_ Xbox 360 profile that has a Xbox LIVE gamertag associated with it

1. Extract the Xbox 360 profile from a hard drive or memory unit using a [FATX](https://free60.org/FATX/) supporting tool
2. Extract `Account` file from the Xbox 360 profile using a [STFS](https://free60.org/STFS/) supporting tool
3. Decrypt the `Account` file using a tool such as `siggggggggggy.exe`
4. Open the decrypted `Account` file in a hex editor such as `HxD`
5. The XUID is located at the offset 0x28, it is 8 bytes in length

https://free60.org/Profile_Account/

------

#### Notes for modders

XUID can be found at offset `0x16653` in _decrypted_ Ninja Gaiden gamesave data

Gamertag can be found at offset `0x167FB` in _decrypted_ Ninja Gaiden gamesave data

Gamesave description can be found at offset `0x1680B` in _decrypted_ Ninja Gaiden or Ninja Gaiden Black gamesave data
