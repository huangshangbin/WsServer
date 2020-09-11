#pragma once


#include "../sha1/sha1.hpp"

#include <iostream>
#include <string>

using namespace std;



class WsEnDecryptUtils {

public:
	static string base64Encode(string srcStr, bool url = false)
	{
		unsigned char const* bytes_to_encode = reinterpret_cast<const unsigned char*>(srcStr.c_str());
		size_t in_len = srcStr.length();

		size_t len_encoded = (in_len + 2) / 3 * 4;

		unsigned char trailing_char = url ? '.' : '=';

		const char* base64_chars[2] = {
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789"
			"+/",

			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789"
			"-_" };
		const char* base64_chars_ = base64_chars[url];

		std::string ret;
		ret.reserve(len_encoded);

		unsigned int pos = 0;

		while (pos < in_len) {
			ret.push_back(base64_chars_[(bytes_to_encode[pos + 0] & 0xfc) >> 2]);

			if (pos + 1 < in_len) {
				ret.push_back(base64_chars_[((bytes_to_encode[pos + 0] & 0x03) << 4) + ((bytes_to_encode[pos + 1] & 0xf0) >> 4)]);

				if (pos + 2 < in_len) {
					ret.push_back(base64_chars_[((bytes_to_encode[pos + 1] & 0x0f) << 2) + ((bytes_to_encode[pos + 2] & 0xc0) >> 6)]);
					ret.push_back(base64_chars_[bytes_to_encode[pos + 2] & 0x3f]);
				}
				else {
					ret.push_back(base64_chars_[(bytes_to_encode[pos + 1] & 0x0f) << 2]);
					ret.push_back(trailing_char);
				}
			}
			else {

				ret.push_back(base64_chars_[(bytes_to_encode[pos + 0] & 0x03) << 4]);
				ret.push_back(trailing_char);
				ret.push_back(trailing_char);
			}

			pos += 3;
		}


		return ret;
	}

	static string base64Decode(string base64Str, bool remove_linebreaks = false)
	{
		if (remove_linebreaks) {

			if (!base64Str.length()) {
				return "";
			}

			std::string copy(base64Str);

			size_t pos = 0;
			while ((pos = copy.find("\n", pos)) != std::string::npos) {
				copy.erase(pos, 1);
			}

			return base64Decode(copy, false);

		}

		size_t length_of_string = base64Str.length();
		if (!length_of_string) return std::string("");

		size_t in_len = length_of_string;
		size_t pos = 0;


		size_t approx_length_of_decoded_string = length_of_string / 4 * 3;
		std::string ret;
		ret.reserve(approx_length_of_decoded_string);

		while (pos < in_len) {

			unsigned int pos_of_char_1 = pos_of_char(base64Str[pos + 1]);

			ret.push_back(static_cast<std::string::value_type>(((pos_of_char(base64Str[pos + 0])) << 2) + ((pos_of_char_1 & 0x30) >> 4)));

			if (base64Str[pos + 2] != '=' && base64Str[pos + 2] != '.') { // accept URL-safe base 64 strings, too, so check for '.' also.

				unsigned int pos_of_char_2 = pos_of_char(base64Str[pos + 2]);
				ret.push_back(static_cast<std::string::value_type>(((pos_of_char_1 & 0x0f) << 4) + ((pos_of_char_2 & 0x3c) >> 2)));

				if (base64Str[pos + 3] != '=' && base64Str[pos + 3] != '.') {
					ret.push_back(static_cast<std::string::value_type>(((pos_of_char_2 & 0x03) << 6) + pos_of_char(base64Str[pos + 3])));
				}
			}

			pos += 4;
		}

		return ret;
	}

	static string sha1Encrypt(string srcStr)
	{
		SHA1 sha1;
		sha1.update(srcStr);

		return sha1.final();
	}

private:
	static unsigned int pos_of_char(const unsigned char chr) 
	{
		if (chr >= 'A' && chr <= 'Z') return chr - 'A';
		else if (chr >= 'a' && chr <= 'z') return chr - 'a' + ('Z' - 'A') + 1;
		else if (chr >= '0' && chr <= '9') return chr - '0' + ('Z' - 'A') + ('z' - 'a') + 2;
		else if (chr == '+' || chr == '-') return 62; // Be liberal with input and accept both url ('-') and non-url ('+') base 64 characters (
		else if (chr == '/' || chr == '_') return 63; // Ditto for '/' and '_'

		throw "If input is correct, this line should never be reached.";
	}
};