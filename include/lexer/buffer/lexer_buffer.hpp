#pragma once

#include <string.h>

#include "lexer/lexer.hpp"
#include "report.hpp"

#define LEXER_BUFFER_SIZE 256

#define CONSUME(check, c) while (check(c)) { this->_buffer[count++] = c;		\
	this->skip(); c = this->peek(); }											\
	ASSERT(count < LEXER_BUFFER_SIZE);

struct Lexer_Buffer {
	Location location;

	char _buffer[LEXER_BUFFER_SIZE];

	virtual char next () = 0;
	virtual bool has_next () = 0;
	virtual char peek (size_t offset = 0) = 0;

	virtual char* get_next_id () {
		char c = this->peek();
	    if (ALPHA(c)) {
			size_t count = 0;

			while (ALPHANUM(c)) {
				this->_buffer[count++] = c;
				this->skip();
				c = this->peek();
			}
			ASSERT(count < LEXER_BUFFER_SIZE);

			this->_buffer[count] = 0;
			return _strdup(this->_buffer);
	    }
	    return NULL;
	}

	virtual char* get_next_string () {
		char c = this->peek();
	    if (c == '"') {
			this->skip();
			size_t count = 0;

			c = this->next();
			while (c != '"') {
				ASSERT(count < LEXER_BUFFER_SIZE);
				if (c == '\\') {
					c = this->next();
					switch (c) {
						case 'n': this->_buffer[count++] = '\n'; break;
						case 't': this->_buffer[count++] = '\t'; break;
						default:  this->_buffer[count++] = c;    break;
					}
				} else this->_buffer[count++] = c;
				c = this->next();
			}
			this->_buffer[count] = 0;

			this->replace_special_characters(this->_buffer, count);
			return _strdup(this->_buffer);
	    } else return NULL;
	}

	virtual char* get_next_number () {
		size_t count = 0;
		char c = this->peek();
		if (c == '0' && this->peek(1) == 'x') {
			this->_buffer[count++] = '0';
			this->_buffer[count++] = 'x';
			this->skip(2);
			c = this->peek();
			CONSUME(ALPHANUM, c)
			this->_buffer[count] = 0;

			return _strdup(this->_buffer);
		} else if (c == '0' && this->peek(1) == 'b') {
			this->_buffer[count++] = '0';
			this->_buffer[count++] = 'b';
			this->skip(2);
			c = this->peek();
			CONSUME(DIGIT, c)
			this->_buffer[count] = 0;

			return _strdup(this->_buffer);
		} else {
		    if (c == '+' || c == '-') {
		        this->_buffer[count++] = c;
				this->skip();
		        c = this->peek();
		    }
		    if (DIGIT(c) || c == '.') {
				CONSUME(DIGIT, c)
		        if (c == '.') {
		            this->_buffer[count++] = c;
		            this->skip();
		            c = this->peek();
					CONSUME(DIGIT, c)
		        }
				this->_buffer[count] = 0;

				return _strdup(this->_buffer);
		    }
		}
	    return NULL;
	}


    bool is_next (char c) {
    	return this->peek(0) == c;
    }

    bool is_next (const char* expected, size_t length) {
    	for (unsigned int i = 0; i < length; i++) {
            if (this->peek(i) != expected[i])
                return false;
        }
        return true;
    }

    void skip (size_t count = 1) {
    	unsigned int i = 0;
    	while (this->has_next() && i < count) {
    		this->next();
    		i += 1;
    	}
    }

    void skip_any (const char* chars) {
    	while (this->has_next()) {
    		char _c = this->peek(0);
    		if (strchr(chars, _c) == NULL) {
    			break;
    		} else this->next();
    	}
    }

    void skip_until (const char* stopper) {
    	unsigned int i = 0;
    	while (this->has_next()) {
    		char _c = this->next();
    		if (_c == stopper[i]) {
    			if (++i == strlen(stopper)) return;
    		} else i = 0;
    	}
    }

    void handle_location (char character) {
    	if (character == EOF) return;
    	if (character == '\n') {
    		this->location.line += 1;
    		this->location.col = 1;
    	} else this->location.col += 1;
    }

	void replace_special_characters (char* _buff, size_t length) {
		for (size_t i = 0; i < length; i++) {
			if (_buff[i] == '\\') {
				switch (_buff[i + 1]) {
					case 'n': _buff[i] = '\n'; 			break;
					case 't': _buff[i] = '\t'; 			break;
					default:  _buff[i] = _buff[i + 1];	break;
				}

				memcpy(&_buff[i + 1], &_buff[i + 2], length);
				i += 1;
			}
		}
	}
};
