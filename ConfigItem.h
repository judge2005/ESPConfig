/*
 * ConfigItem.h
 *
 *  Created on: Dec 6, 2017
 *      Author: Paul
 */

#ifndef LIBRARIES_CONFIGS_CONFIGITEM_H_
#define LIBRARIES_CONFIGS_CONFIGITEM_H_
#include "Arduino.h"
#include <EEPROM.h>

#ifndef DEBUG
#define DEBUG(...) {}
#endif

struct BaseConfigItem {
	BaseConfigItem(const char *name, int maxSize) :
		name(name),
		maxSize(maxSize),
		start(-1)
	{}
	virtual ~BaseConfigItem() {}

	virtual int init(int start) { this->start = start; return start + maxSize; }
	virtual unsigned int getChecksum(int index) {
		unsigned int checksum = 0;
		for (int i=0; name[i] != 0; i++) {
			checksum += name[i] ^ index;
		}
		return checksum;
	}
	virtual BaseConfigItem* get(const char *name) {
		if (strcmp(name, this->name) == 0) {
			return this;
		}

		return 0;
	}

	virtual void fromString(const String &s) = 0;
	virtual void put() const = 0;
	virtual BaseConfigItem& get() = 0;
	virtual String toJSON(bool bare = false) const = 0;
	virtual void debug(Print *debugPrint) const = 0;
	const char *name;
	byte maxSize;
	int start;
};

template<typename T>
struct ConfigItem : public BaseConfigItem {
	ConfigItem(const char *name, const byte maxSize, const T value)
	: BaseConfigItem(name, maxSize), value(value)
	{}

	T value;

	virtual void put() const { EEPROM.put(start, value); }
	virtual BaseConfigItem& get() { EEPROM.get(start, value); return *this; }
	virtual void debug(Print *debugPrint) const;
	operator T () const { return value; }
};

struct BooleanConfigItem : public ConfigItem<bool> {
	BooleanConfigItem(const char *name, const bool value)
	: ConfigItem(name, sizeof(bool), value)
	{}

	virtual void fromString(const String &s) { value = s.equalsIgnoreCase("true") ? 1 : 0; }
	virtual String toJSON(bool bare = false) const { return value ? "true" : "false"; }
	BooleanConfigItem& operator=(const bool val) { value = val; return *this; }
};

struct ByteConfigItem : public ConfigItem<byte> {
	ByteConfigItem(const char *name, const byte value)
	: ConfigItem(name, sizeof(byte), value)
	{}

	virtual void fromString(const String &s) { value = s.toInt(); }
	virtual String toJSON(bool bare = false) const { return String(value); }
	ByteConfigItem& operator=(const byte val) { value = val; return *this; }
};

struct IntConfigItem : public ConfigItem<int> {
	IntConfigItem(const char *name, const int value)
	: ConfigItem(name, sizeof(int), value)
	{}

	virtual void fromString(const String &s) { value = s.toInt(); }
	virtual String toJSON(bool bare = false) const { return String(value); }
	IntConfigItem& operator=(const int val) { value = val; return *this; }
};

struct StringConfigItem : public ConfigItem<String> {
	StringConfigItem(const char *name, const byte maxSize, const String &value)
	: ConfigItem(name, maxSize, value)
	{}

	virtual void fromString(const String &s) { value = s; }
	virtual String toJSON(bool bare = false) const { return "\"" + value + "\""; }
	virtual void put() const;
	virtual BaseConfigItem& get();
	StringConfigItem& operator=(const String &val) { value = val; return *this; }
};

/**
 * TODO: parse JSON to populate all items!
 */
struct CompositeConfigItem : public BaseConfigItem {
	CompositeConfigItem(const char *name, const byte maxSize, BaseConfigItem** value)
	: BaseConfigItem(name, maxSize), value(value)
	{}

	BaseConfigItem** value;	// an array of pointers to BaseConfigItems

	virtual int init(int start);
	virtual BaseConfigItem* get(const char *name);
	virtual unsigned int getChecksum(int index);

	virtual void fromString(const String &s) {  }
	virtual String toJSON(bool bare = false) const;
	virtual void put() const;
	virtual BaseConfigItem& get();
	CompositeConfigItem& operator=(BaseConfigItem** val) { value = val; return *this; }
	virtual void debug(Print *debugPrint) const;
};

#endif /* LIBRARIES_CONFIGS_CONFIGITEM_H_ */
