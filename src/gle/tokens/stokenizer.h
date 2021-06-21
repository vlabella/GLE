/************************************************************************
 *                                                                      *
 * GLE - Graphics Layout Engine <http://www.gle-graphics.org/>          *
 *                                                                      *
 * Modified BSD License                                                 *
 *                                                                      *
 * Copyright (C) 2009 GLE.                                              *
 *                                                                      *
 * Redistribution and use in source and binary forms, with or without   *
 * modification, are permitted provided that the following conditions   *
 * are met:                                                             *
 *                                                                      *
 *    1. Redistributions of source code must retain the above copyright *
 * notice, this list of conditions and the following disclaimer.        *
 *                                                                      *
 *    2. Redistributions in binary form must reproduce the above        *
 * copyright notice, this list of conditions and the following          *
 * disclaimer in the documentation and/or other materials provided with *
 * the distribution.                                                    *
 *                                                                      *
 *    3. The name of the author may not be used to endorse or promote   *
 * products derived from this software without specific prior written   *
 * permission.                                                          *
 *                                                                      *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR   *
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED       *
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE   *
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY       *
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL   *
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE    *
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS        *
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER *
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR      *
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN  *
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                        *
 *                                                                      *
 ************************************************************************/

#include <ctype.h>

/*
 * A simple tokenizer
 */

enum empty_token_policy { drop_empty_tokens, keep_empty_tokens };

class char_separator {
public:
	explicit char_separator(const char* dropped_delims,
	                        const char* kept_delims = 0,
	                        empty_token_policy empty_tokens = drop_empty_tokens)
	: m_dropped_delims(dropped_delims),
	  m_use_ispunct(false),
	  m_use_isspace(false),
	  m_empty_tokens(empty_tokens),
	  m_output_done(false) {
		if (kept_delims) m_kept_delims = kept_delims;
	}

	explicit char_separator()
	: m_use_ispunct(true),
	  m_use_isspace(true),
	  m_empty_tokens(drop_empty_tokens) {
	}

	bool next(string::const_iterator& next, string::const_iterator& end, string& tok) {
		tok = "";
		// skip past all dropped_delims
		if (m_empty_tokens == drop_empty_tokens) {
			for (; next != end  && is_dropped(*next); ++next) {
			}
		}
		if (m_empty_tokens == drop_empty_tokens) {
			if (next == end) return false;
			// if we are on a kept_delims move past it and stop
			if (is_kept(*next)) {
				tok += *next;
				++next;
			} else {
				// append all the non delim characters
				for (; next != end && !is_dropped(*next) && !is_kept(*next); ++next)
					tok += *next;
			}
		} else { // m_empty_tokens == keep_empty_tokens
			// Handle empty token at the end
			if (next == end) {
				if (m_output_done == false) {
					m_output_done = true;
					return true;
				} else {
					return false;
				}
			}
			if (is_kept(*next)) {
				if (m_output_done == false) {
					m_output_done = true;
				} else {
					tok += *next;
					++next;
					m_output_done = false;
				}
			} else if (m_output_done == false && is_dropped(*next)) {
				m_output_done = true;
			} else {
				if (is_dropped(*next)) ++next;
				for (; next != end && !is_dropped(*next) && !is_kept(*next); ++next)
					tok += *next;
				m_output_done = true;
			}
		}
		return true;
	}

private:
	string m_kept_delims;
	string m_dropped_delims;
	bool m_use_ispunct;
	bool m_use_isspace;
	empty_token_policy m_empty_tokens;
	bool m_output_done;

	bool is_kept(char E) const {
		if (m_kept_delims.length()) {
			return m_kept_delims.find(E) != string::npos;
		} else if (m_use_ispunct) {
			return ispunct(E) != 0;
		} else {
			return false;
		}
	}

	bool is_dropped(char E) const {
		if (m_dropped_delims.length()) {
			return m_dropped_delims.find(E) != string::npos;
		} else if (m_use_isspace) {
			return isspace(E) != 0;
		} else {
			return false;
		}
	}
};

class level_char_separator {
public:
	explicit level_char_separator(const char* dropped_delims,
	                              const char* kept_delims,
				      const char* level_up,
				      const char* level_down)
	        : m_dropped_delims(dropped_delims),
		  m_kept_delims(kept_delims),
		  m_level_up_delims(level_up),
		  m_level_down_delims(level_down) {
	}

	bool next(string::const_iterator& next, string::const_iterator& end, string& tok) {
		tok = "";
		while (next != end && is_dropped(*next)) {
			++next;
		}
		if (next == end) return false;
		if (is_kept(*next)) {
			tok += *next;
			++next;
		} else {
			// append all the non delim characters
			int level = 0;
			while (next != end) {
				if (level == 0) {
					if (is_dropped(*next) || is_kept(*next)) break;
					if (is_level_up(*next)) level++;
					tok += *next;
				} else {
					if (is_level_down(*next)) level--;
					else if (is_level_up(*next)) level++;
					tok += *next;
				}
				++next;
			}
		}
		return true;
	}

private:
	string m_dropped_delims;
	string m_kept_delims;
	string m_level_up_delims;
	string m_level_down_delims;

	bool is_level_up(char E) const {
		return m_level_up_delims.find(E) != string::npos;
	}

	bool is_level_down(char E) const {
		return m_level_down_delims.find(E) != string::npos;
	}

	bool is_kept(char E) const {
		if (m_kept_delims.length()) {
			return m_kept_delims.find(E) != string::npos;
		} else {
			return false;
		}
	}

	bool is_dropped(char E) const {
		if (m_dropped_delims.length()) {
			return m_dropped_delims.find(E) != string::npos;
		} else {
			return false;
		}
	}
};

// A view of a tokenized "sequence"
template < typename TokenizerFunc = char_separator > class tokenizer {
public:
	tokenizer(const string& input, TokenizerFunc& token_func) : m_token_func(token_func) {
		m_input = input;
		m_current = m_input.begin();
		m_end = m_input.end();
		m_more = m_token_func.next(m_current, m_end, m_next_token);
	}

	tokenizer(TokenizerFunc& token_func) : m_token_func(token_func) {
	}

	void set_input(const char* input) {
		m_input = input;
		m_current = m_input.begin();
		m_end = m_input.end();
		m_more = m_token_func.next(m_current, m_end, m_next_token);
	}

	void set_input(const string& input) {
		m_input = input;
		m_current = m_input.begin();
		m_end = m_input.end();
		m_more = m_token_func.next(m_current, m_end, m_next_token);
	}

	bool has_more() {
		return m_more;
	}

	const string& next_token() {
		m_cr_token = m_next_token;
		m_more = m_token_func.next(m_current, m_end, m_next_token);
		return m_cr_token;
	}

	const string& cr_token() {
		return m_next_token;
	}
private:
	bool m_more;
	string m_cr_token, m_next_token, m_input;
	string::const_iterator m_current, m_end;
	TokenizerFunc& m_token_func;
};
