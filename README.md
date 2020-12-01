# ESPConfig
At a high level this provides a polymorphic set of classes called ConfigItems. They contain a name, a value and a size and can be used wherever a native type would be expected. In addition there is a CompositeConfigItem that is a collection of other config items.

The class EEPROMConfig is initialized with a single config item (remember, they can be composite). It creates a checksum of the passed config item from the names, sizes and positions of all the config items it contains. It compares it with a checksum from the EEPROM and if they are different, it will initialize the EEPROM with the passed item. It modifies all the passed items with an offset that is used when writing their values to EEPROM.

Config items are responsible for writing their own values to EEPROM. e.g.:

## Example
```c++
ByteConfigItem anItem("an_item", 6);
EEPROMConfig config(anItem);
anItem = 20;
anItem.put();
// For ESPxxxx
config.commit();
```
