#include "../../common/util.h"
#include "Explanation_parser.h"

namespace {

enum class TokenType {
  tag,
  digits,
  dash,
  other,
  eof,
};

struct Token {
  TokenType type;
  const char *begin;
  const char *end;

  size_t size() const { return end - begin; }

  operator string_view() const { return string_view(begin, size()); }

  operator string() const { return string(begin, end); }

  operator WString() const { return WString(operator string()); }
};

class Lexer {
public:
  explicit Lexer(string_view source)
      : begin_(source.begin()), end_(source.end()) {}

  Token next();

  void unnext(Token);

private:
  Token real_next_();

  using Ptr = const char *;
  Ptr begin_, end_;
  std::vector<Token> stack_;
};

void Lexer::unnext(Token tok) { stack_.push_back(tok); }

} // namespace

void parse_explanation(string_view source, Explanation_writer &writer,
                       Code_highlighter &highlighter) {
  Lexer lexer(source);

  for (;;) {
    Token tok = lexer.next();

    if (tok.type == TokenType::eof) {
      return;
    } else if (tok.type == TokenType::tag) {
      int line = writer.link(tok);

      Token tok1 = lexer.next();
      if (tok1.type != TokenType::dash) {
        lexer.unnext(tok1);
        highlighter.highlight(line);
        continue;
      }

      Token tok2 = lexer.next();
      if (tok2.type != TokenType::tag && tok2.type != TokenType::digits) {
        lexer.unnext(tok2);
        lexer.unnext(tok1);
        highlighter.highlight(line);
        continue;
      }

      writer.plain_string(tok1);

      int line2 = writer.link(tok2);
      highlighter.highlight(line, line2);
    } else {
      writer.plain_string(tok);
    }
  }
}

Token Lexer::next() {
  if (stack_.empty()) {
    return real_next_();
  } else {
    Token result = stack_.back();
    stack_.pop_back();
    return result;
  }
}

Token Lexer::real_next_() {
  Token tok{TokenType::other, begin_, begin_};

  auto at_eof = [&]() -> bool { return tok.end == end_; };

  auto consumed = [&]() -> ssize_t { return tok.end - tok.begin; };

  auto next = [&](int (*pred)(int)) -> bool {
    if (tok.end + 1 < end_) {
      return pred(tok.end[1]);
    } else {
      return false;
    }
  };

  auto consume = [&]() -> void {
    assert(!at_eof());
    ++tok.end;
  };

  auto it = [&]() -> char {
    assert(!at_eof());
    return *tok.end;
  };

  auto skip_while = [&](int (*skippable)(int)) -> void {
    while (!at_eof() && skippable(it())) {
      consume();
    }
  };

  /////
  /////
  /////

  if (at_eof()) {
    tok.type = TokenType::eof;
    return tok;
  }

  do {
    if (isdigit(it())) {
      if (consumed())
        break;

      tok.type = TokenType::digits;
      consume();
      skip_while(isdigit);
      skip_while(isspace);

      break;
    }

    if (it() == '-') {
      if (consumed())
        break;

      tok.type = TokenType::dash;
      consume();
      skip_while(isspace);

      break;
    }

    if ((it() == 'L' || it() == 'l' || it() == '#') && next(isdigit)) {
      if (consumed())
        break;

      tok.type = TokenType::tag;
      consume();
      consume();
      skip_while(isdigit);
      skip_while(isspace);

      break;
    }

    consume();
  } while (tok.end < end_);

  begin_ = tok.end;

  return tok;
}

void Code_highlighter::highlight(int from, int to) {
  highlight(from);
  highlight(to);
  while (++from < to)
    highlight(from);
}

void Explanation_writer::plain_string(std::string_view sv) {
  for (char c : sv)
    plain_char(c);
}

void Explanation_writer::plain_range(const char *begin, const char *end) {
  plain_string(string_view(begin, end - begin));
}

WString Explanation_writer::wstr() const { return WString(os_.str()); }

void Explanation_writer::raw_text_(char c) { os_ << c; }

void Explanation_writer::raw_text_(string_view sv) { os_ << sv; }
