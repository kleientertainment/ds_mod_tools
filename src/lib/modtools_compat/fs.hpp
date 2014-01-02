/*
 * Header-only library abstracting some filesystem properties.
 */

#ifndef MODTOOLS_COMPAT_FS_HPP
#define MODTOOLS_COMPAT_FS_HPP

#include "modtools_compat/common.hpp"
#include "modtools_compat/posix.hpp"

#include <string>
#include <algorithm>
#include <iterator>
#include <cassert>

namespace Compat {
#ifdef IS_UNIX
#	define DIR_SEP_MACRO '/'
#	define DIR_SEP_STR_MACRO "/"
#else
#	define DIR_SEP_MACRO '\\'
#	define DIR_SEP_STR_MACRO "\\"
#endif

	static const char DIRECTORY_SEPARATOR = DIR_SEP_MACRO;	

	static const char DUAL_DIRECTORY_SEPARATOR = (DIRECTORY_SEPARATOR == '/' ? '\\' : '/');

	class Path : public std::string {
	public:
		typedef struct stat stat_t;

		static const char SEPARATOR = DIRECTORY_SEPARATOR;

		std::string& toString() {
			return static_cast<std::string&>(*this);
		}

		const std::string& toString() const {
			return static_cast<const std::string&>(*this);
		}

	private:
		static void normalizeDirSeps(std::string& str, size_t offset = 0) {
			std::string::iterator begin = str.begin();
			std::advance(begin, offset);
			std::replace(begin, str.end(), DUAL_DIRECTORY_SEPARATOR, DIRECTORY_SEPARATOR);
		}

		/*
		 * Skips extra slashes in a normalized string.
		 * Receives begin and end iterators, modifying them
		 * to the actual range.
		 */
		static void skip_extra_slashes(std::string::const_iterator& begin, std::string::const_iterator& end) {
			if(begin == end)
				return;

			const size_t len = static_cast<size_t>( std::distance(begin, end) );
			const std::string::const_iterator real_end = end;

			// It points to the last character, for now.
			end = begin;
			std::advance(end, len - 1);

			for(; begin != real_end && *begin == DIRECTORY_SEPARATOR; ++begin);
			for(; end != begin && *end == DIRECTORY_SEPARATOR; --end);
			++end;
		}


		int inner_stat(stat_t& buf) const {
			return ::stat(this->c_str(), &buf);
		}


		size_t get_extension_position() const {
			const size_t dot_pos = find_last_of('.');
			if(dot_pos == npos || dot_pos < find_last_of(SEPARATOR)) {
				return npos;
			}
			else {
				return dot_pos + 1;
			}
		}
		
	public:
		void stat(stat_t& buf) const {
			if(inner_stat(buf)) {
				buf = stat_t();
			}
		}

		stat_t stat() const {
			stat_t buf;
			stat(buf);
			return buf;
		}


		/*
		 * Appends a string, preceded by a directory separator if necessary.
		 */
		Path& appendPath(std::string str) {
			if(str.length() == 0)
				return *this;

			normalizeDirSeps(str);

			const size_t old_len = this->length();

			if(old_len != 0 || str[0] == DIRECTORY_SEPARATOR) {
				std::string::append(1, DIRECTORY_SEPARATOR);
			}
			std::string::const_iterator begin, end;
			begin = str.begin();
			end = str.end();
			skip_extra_slashes(begin, end);

			std::string::append(begin, end);
			return *this;
		}

		Path& assignPath(const std::string& str) {
			std::string::clear();
			return appendPath(str);
		}

		Path& operator=(const std::string& str) {
			return assignPath(str);
		}

		Path& operator=(const char *str) {
			return assignPath(str);
		}

		Path& operator=(const Path& p) {
			std::string::assign(p);
			return *this;
		}

		Path(const std::string& str) {
			*this = str;
		}

		Path(const char* str) {
			*this = str;
		}

		Path(const Path& p) {
			*this = p;
		}

		Path() {}

		Path copy() const {
			return Path(*this);
		}

		Path& operator+=(const std::string& str) {
			std::string::append(str);
			return *this;
		}

		Path& operator+=(const char *str) {
			std::string::append(str);
			return *this;
		}

		Path operator+(const std::string& str) const {
			return this->copy() += str;
		}

		Path operator+(const char *str) const {
			return this->copy() += str;
		}

		Path& operator/=(const std::string& str) {
			return appendPath(str);
		}

		Path& operator/=(const char *str) {
			return appendPath(str);
		}

		Path operator/(const std::string& str) const {
			return this->copy() /= str;
		}

		Path operator/(const char *str) const {
			return this->copy() /= str;
		}

		/*
		 * Splits the path into directory and file parts.
		 *
		 * The "directory part" accounts for everything until the last
		 * separator (not including the separator itself). If no separator
		 * exists, the directory will be ".".
		 *
		 * The directory part DOES NOT include a trailing slash.
		 */
		void split(std::string& dir, std::string& base) const {
			assert( length() > 0 );

			const size_t last_sep = find_last_of(DIRECTORY_SEPARATOR);

			if(last_sep == 0 and length() == 1) {
				// Root directory (so Unix).
				dir = "/";
				base = "/";
				return;
			}

			if(last_sep == npos) {
				dir = ".";
				base = *this;
			}
			else {
				dir = substr(0, last_sep);
				base = substr(last_sep + 1);
			}
		}

		Path dirname() const {
			Path dir, base;
			split(dir, base);
			return dir;
		}

		// With trailing slash.
		Path dirnameWithSlash() const {
			Path p = dirname();
			if(p != "/") {
				p.append(1, DIRECTORY_SEPARATOR);
			}
			return p;
		}

		Path basename() const {
			Path dir, base;
			split(dir, base);
			return base;
		}

		std::string getExtension() const {
			const size_t start = get_extension_position();
			if(start == npos) {
				return "";
			}
			else {
				return substr(start);
			}
		}

		Path& replaceExtension(const std::string& newext) {
			const size_t start = get_extension_position();
			if(start == npos) {
				std::string::append(1, '.');
			}
			else {
				std::string::erase(start);
			}
			std::string::append(newext);
			return *this;
		}

		Path& removeExtension() {
			const size_t start = get_extension_position();
			if(start != npos) {
				assert( start >= 1 );
				std::string::erase(start - 1);
			}
		}

		bool exists() const {
			stat_t buf;
			return inner_stat(buf) == 0;
		}

		bool isNewerThan(const Path& p) const {
			stat_t mybuf, otherbuf;
			stat(mybuf);
			p.stat(otherbuf);
			return mybuf.st_mtime < otherbuf.st_mtime;
		}
	};
}

#endif
