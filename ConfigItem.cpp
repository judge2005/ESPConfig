/*
 * Configcpp
 *
 *  Created on: Dec 6, 2017
 *      Author: Paul
 */

#include <ConfigItem.h>

template <class T>
void ConfigItem<T>::debug(Print *debugPrint) const {
	if (debugPrint != 0) {
		debugPrint->print(name);
		debugPrint->print(":");
		debugPrint->print(value);
		debugPrint->print(" (");
		debugPrint->print(maxSize);
		debugPrint->println(")");
	}
}

template void ConfigItem<byte>::debug(Print *debugPrint) const;
template void ConfigItem<bool>::debug(Print *debugPrint) const;
template void ConfigItem<int>::debug(Print *debugPrint) const;
template void ConfigItem<String>::debug(Print *debugPrint) const;

void StringConfigItem::put() const {
	int end = start + maxSize;
	for (int i = start; i < end; i++) {
		if (i - start < value.length()) {
			EEPROM.write(i, value[i - start]);
		} else {
			EEPROM.write(i, 0);
			break;
		}
	}
}

BaseConfigItem& StringConfigItem::get() {
	value = String();
	value.reserve(maxSize+1);
	int end = start + maxSize;
	for (int i = start; i < end; i++) {
		byte readByte = EEPROM.read(i);
		if (readByte > 0 && readByte < 128) {
			value += char(readByte);
		} else {
			break;
		}
	}

	return *this;
}

void CompositeConfigItem::debug(Print *debugPrint) const {
	if (debugPrint != 0) {
		debugPrint->print(name);
		debugPrint->print(": {");
		char *sep = "";
		for (int i=0; value[i] != 0; i++) {
			debugPrint->print(sep);
			value[i]->debug(debugPrint);
			sep = ",";
		}
		debugPrint->print("}");
	}
}

String CompositeConfigItem::toJSON(bool bare) const {
	String json;
	json.reserve(200);

	if (!bare) {
		json.concat("{");
	}
	char *sep = "";
	for (int i=0; value[i] != 0; i++) {
		json.concat(sep);
		json.concat("\"");
		json.concat(value[i]->name);
		json.concat("\"");
		json.concat(":");
		json.concat(value[i]->toJSON());
		sep = ",";
	}
	if (!bare) {
		json.concat("}");
	}

	return json;
}

void CompositeConfigItem::put() const {
	for (int i=0; value[i] != 0; i++) {
		value[i]->put();
	}
}

BaseConfigItem& CompositeConfigItem::get() {
	for (int i=0; value[i] != 0; i++) {
		value[i]->get();
	}

	return *this;
}

int CompositeConfigItem::init(int start) {
	this->start = start;

	for (int i=0; value[i] != 0; i++) {
		start = value[i]->init(start);
	}

	this->maxSize = start - this->start;

	return start;
}

BaseConfigItem* CompositeConfigItem::get(const char *name) {
	if (strcmp(name, this->name) == 0) {
		return this;
	}

	for (int i=0; value[i] != 0; i++) {
		BaseConfigItem *itemP = value[i]->get(name);
		if (itemP != 0) {
			return itemP;
		}
	}

	return 0;
}

unsigned int CompositeConfigItem::getChecksum(int index) {
	unsigned int checksum = BaseConfigItem::getChecksum(index);

	for (int i=0; value[i] != 0; i++) {
		checksum += value[i]->getChecksum(index + i) ^ index;
	}

	return checksum;
}
