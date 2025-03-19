/*
  proc_re.cpp
  S. Edward Dolan
  Sunday, July  2 2023
*/

// ... proc re str]
// ... proc re str result]
case vasm::RE_MATCH_2: {
    Regex* re = cpRegex(ppeek(1));
    String* s = cpString(ppeek());
    std::smatch cm;
    if (std::regex_match(s->val(), cm, re->re())) {
        Vector* v = Vector::create();
        for (size_t i=0; i<cm.size(); ++i)
            v->conj(String::fetch(cm[i]));
        ppush(v);
    }
    else
        ppush(NIL);
    break;
}
// ... proc re str int]
// ... proc re str int result]
case vasm::RE_MATCH_3: {
    const std::regex& re = cpRegex(ppeek(2))->re();
    const std::string& str = cpString(ppeek(1))->val();
    int i = cpInteger(ppeek())->val();
    if (i < 0 || i >= static_cast<int>(str.size())) {
        std::stringstream ss;
        ss << "re-match start index (" << i << ") is out of bounds";
        throw SxOutOfBoundsError(ss.str());
    }
    std::smatch cm;
    const std::string& subs = str.substr(i);
    if (std::regex_match(subs, cm, re)) {
        Vector* v = Vector::create();
        for (size_t i=0; i<cm.size(); ++i)
            v->conj(String::fetch(cm[i]));
        ppush(v);
    }
    else
        ppush(NIL);
    break;
}
// ... proc re str int int]
// ... proc re str int int result]
case vasm::RE_MATCH_4: {
    const std::regex& re = cpRegex(ppeek(3))->re();
    const std::string& str = cpString(ppeek(2))->val();
    int i = cpInteger(ppeek(1))->val();
    int j = cpInteger(ppeek())->val();
    if (i < 0 || i >= static_cast<int>(str.size())) {
        std::stringstream ss;
        ss << "re-match start index (" << i << ") is out of bounds";
        throw SxOutOfBoundsError(ss.str());
    }
    if (j <= i)  {
        std::stringstream ss;
        ss << "re-match end index (" << j << ") must be > start index";
        throw SxOutOfBoundsError(ss.str());
    }
    if (j >= static_cast<int>(str.size())) {
        std::stringstream ss;
        ss << "re-match end index (" << j << ") is out of bounds";
        throw SxOutOfBoundsError(ss.str());
    }
    std::smatch cm;
    std::string subs = str.substr(i, j-i);
    if (std::regex_match(subs, cm, re)) {
        Vector* v = Vector::create();
        for (size_t i=0; i<cm.size(); ++i)
            v->conj(String::fetch(cm[i]));
        ppush(v);
    }
    else
        ppush(NIL);
    break;
}
// ... re]
// ... re str]
case vasm::RE_PATTERN_1: {
    ppush(String::fetch(cpRegex(ppeek())->pat()));
    break;
}
