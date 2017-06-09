#include "BigGuys.h"
#include <stdio.h>

#define CHECK(some_str) { for(size_t i = 0; i < strlen(some_str); i++) {if (strchr("abcdefABCDEF1234567890", some_str[i]) == NULL) throw std::runtime_error("invalid character in string: non-hex value");}}
#define BASE_SIZE sizeof(T)
#define SYM_GROUP (BASE_SIZE<<1)
#define MAX_VAL ((1<<(sizeof(T)<<3)) - 1)
#define BASE (((size_t)MAX_VAL)+1)

template<typename T>
BigGuys<T>::BigGuys(size_t cap) {
    len = 0;
    guy = new T[cap];
    memset(guy, 0, cap * BASE_SIZE);
}

template<typename T>
BigGuys<T>::BigGuys(BigGuys<T> const &src) {
    cap = src.cap;
    guy = new T[cap];
    memcpy(guy, src.guy, cap * BASE_SIZE);
    len = src.len;
}

template<typename T>
BigGuys<T>::BigGuys(string const src) {
    CHECK(src.c_str());

    if (src.length() == 0) {
        len = 0;
        cap = 0;
        guy = nullptr;
    } else {
        cap = src.length() / SYM_GROUP + 1;
        len = 0;
        guy = new T[cap];

        //string padding to full SYM_GROUP
        short int pad = src.length() % SYM_GROUP ? SYM_GROUP - src.length() % SYM_GROUP : 0;
        char* temp = new char[src.length() + pad + 1];
        memset(temp, '0', src.length() + pad);
        strcpy(temp+pad, src.c_str());

        for( long int i = strlen(temp)-SYM_GROUP; i >= 0; i-=SYM_GROUP) {
            T num = (T) strtoul(&temp[i], NULL, 16);
            guy[len++] = num;
            temp[i] = 0;
        }

        delete[] temp;
    }
    clear_insig();
}

template <typename T>
void BigGuys<T>::clear_insig() {
    //std::cout << "DEBUG START\n";
    if (len == 0)
        return;
    //printf("%p\n", guy);
    long int i;
    for(i = len - 1; i >= 0 && guy[i] == 0; i--);

    if (i == -1) {
        delete[] guy;
        guy = new T[1];
        guy[0] = 0;
        len = 1;
        cap = 1;
    } else {
        T* tmp = new T[i+1];
        for(long int j = 0; j <= i; j++)
            tmp[j] = guy[j];
        delete[] guy;
        guy = tmp;
        cap = i+1;
        len = i+1;
    }
    //std::cout << "DEBUG END\n";
}

template <typename T>
BigGuys<T>& BigGuys<T>::operator= (BigGuys<T> const& src) {
    delete[] guy;
    cap = src.cap;
    guy = new T[cap];
    memcpy(guy, src.guy, cap * BASE_SIZE);
    len = src.len;

    return *this;
}

template <typename T>
BigGuys<T> BigGuys<T>::operator+ (BigGuys<T> const & add) {
    size_t max_cap = cap > add.cap ? cap : add.cap;
    BigGuys<T> tmp(max_cap + 1);

    size_t OF = 0, i;
    for (i = 0; i < len && i < add.len; i++) {
        tmp.len++;
        tmp[i] = (guy[i] + add[i] + OF) & MAX_VAL;
        OF = (guy[i] + add[i] + OF) / (MAX_VAL + 1);
    }

    if (i == len && i < add.len) {
        for(; i < add.len; i++) {
            tmp.len++;
            tmp[i] = (add[i] + OF) & MAX_VAL;
            OF = (add[i] + OF) / (MAX_VAL + 1);
        }
    } else if (i == add.len && i < len) {
        for(; i < len; i++) {
            tmp.len++;
            tmp[i] = (guy[i] + OF) & MAX_VAL;
            OF = (guy[i] + OF) / (MAX_VAL + 1);
        }
    }
    tmp.len++;
    tmp[i] = OF;

    tmp.clear_insig();
    return tmp;
}

template <typename T>
BigGuys<T> BigGuys<T>::operator- (BigGuys<T> const & sub) {
    size_t max_cap = cap > sub.cap ? cap : sub.cap;
    BigGuys<T> tmp(max_cap);

    size_t OF = 0, i;
    for(i = 0; i < sub.len; i++) {
        tmp.len++;
        tmp[i] = (guy[i] - sub[i] - OF) & MAX_VAL;
        OF = (MAX_VAL - (guy[i] - sub[i] - OF)) / (MAX_VAL + 1);
    }

    if (i < len) {
        for(; i < len; i++) {
            tmp.len++;
            tmp[i] = (guy[i] - OF) & MAX_VAL;
            OF = (MAX_VAL - (guy[i] - OF)) / (MAX_VAL + 1);
        }
    }

    tmp.clear_insig();
    return tmp;
}

template <typename T>
BigGuys<T> BigGuys<T>::operator* (BigGuys<T> const & mul) {
    BigGuys<T> tmp(cap + mul.cap);
    tmp.len = len + mul.len;

    size_t OF, i, j;
    for(i = 0; i < len; i++) {
        if (guy[i] == 0)
            continue;

        for(j = 0, OF = 0; j < mul.len; j++) {
            size_t t = (size_t)guy[i] * (size_t)mul[j] + (size_t)tmp[i+j] + OF;
            tmp[i+j] = t & MAX_VAL;
            OF = t / (MAX_VAL + 1);
        }
        tmp[tmp.len - len + i] = OF;
    }

    tmp.clear_insig();
    return tmp;
}

template <typename T>
BigGuys<T> BigGuys<T>::mul_base(T mul) {
    BigGuys<T> tmp(cap + 1);
    tmp.len = len + 1;

    size_t OF, i;
    for(i = 0, OF = 0; i < len; i++) {
        size_t t = (size_t)guy[i] * mul + OF;
        tmp[i] = t & MAX_VAL;
        OF = t / (MAX_VAL + 1);
    }
    tmp[i] = OF;

    tmp.clear_insig();
    return tmp;
}

template <typename T>
std::tuple<BigGuys<T>, T> BigGuys<T>::div_base(T diver) {
    BigGuys<T> tmp(cap);
    tmp.len = len;

    long unsigned int r = 0, t = 0;
    for(int i = len-1; i >= 0; i--) {
        t = guy[i] + r * (MAX_VAL + 1);
        tmp[i] = t / diver;
        r = t % diver;
    }

    tmp.clear_insig();

    return std::make_tuple(tmp, (T)r);
}

template <typename T>
std::tuple<BigGuys<T>, BigGuys<T>> BigGuys<T>::operator/ (BigGuys<T> const & diver) {

    int diff_len = len - diver.get_len();
    BigGuys<T> this_clone, diver_clone;
    this_clone = *this;
    diver_clone = diver;

    if ((diver.get_len() == 0) || (diver.get_len() == 1) && (diver[0] == 0))
        throw std::runtime_error("Dividing by zero or None");

    if (diver > this_clone) {
        BigGuys<T> tmp("0");
        return std::make_tuple(tmp, this_clone);
    }

    if (diver.get_len() == 1) {
        auto kek = this_clone.div_base(diver[0]);
        BigGuys<T> res(1), some_temp;
        res.len = 1;
        res[0] = std::get<1>(kek);
        some_temp = std::get<0>(kek);
        res.clear_insig();
        some_temp.clear_insig();
        
        return std::make_tuple(some_temp, res);
    }

    BigGuys<T> res(diff_len+1), mod(diff_len+1);
    res.len = diff_len+1;
    mod.len = diff_len+1;

    size_t d = BASE / (diver[diver.len - 1] + 1);
        //may be shoud to add same thing as in `else` branch
    this_clone = this_clone.mul_base(d);
    diver_clone = diver_clone.mul_base(d);

    BigGuys<T> buf(len + 1);
    buf.len = len + 1;
    memcpy(buf.guy, guy, len * BASE_SIZE);

    bool is_negative;
    auto tlen = diver_clone.get_len();
    for(int j = diff_len; j >= 0; j--) {

        BigGuys temp(tlen+1);
        is_negative = false;
        temp.len = tlen + 1;
        //memcpy(temp.guy, guy, diver.get_len() * BASE_SIZE);
        //is_negative = false;

        int long step_3 = (buf[j+tlen] * BASE + buf[j+tlen-1]) / diver_clone[tlen-1];
        //Alexander gave me some advice about arithmetic
        int long advice = (buf[j+tlen] * BASE + buf[j+tlen-1]) - step_3 * diver_clone[tlen-1];
        while (((step_3 * diver_clone[tlen-2]) > (advice * BASE + guy[j+tlen-2])) && (advice < BASE)) {
            step_3--;
            advice += diver_clone[tlen-1];
        }

        for(int i = tlen; i >= 0; i--) {
            temp[i] = buf[i+j]; 
        }
        temp.clear_insig();

        if (!(diver_clone.mul_base(step_3) > temp)) {
            temp = temp - diver_clone.mul_base(step_3);
        } else {
            BigGuys<T> another_temporary_var(tlen+2);
            another_temporary_var.len = tlen + 2;
            another_temporary_var[tlen+1] = 1;
            temp = (another_temporary_var - diver_clone.mul_base(step_3)) + temp;
            is_negative = true;
        }


        mod[j] = step_3;
        if (is_negative) {
            mod[j]--;
            temp = temp + diver_clone;
        }
        
        std::cout << "Before for\n";        
        for(int i = tlen; i >= 0; i--) {
            if ( i < temp.len ) {
                buf[j+i] = temp[i];
            } else {
                buf[j+i] = 0;
            }
        }
        std::cout << "Here is no after sorry\n";
        //std::cout << "before buf\n";
        buf.clear_insig();
        //std::cout << "after buf\n";
    }

    mod.len = tlen+1;
    mod.clear_insig();
    res = std::get<0>(buf.div_base(d));
    res.clear_insig();

    return std::make_tuple(res, mod);
}

template <typename T>
BigGuys<T> BigGuys<T>::power(size_t const pw) const {
    BigGuys<T> tmp("1");

    for(size_t i = 1; i <= pw; i++)
        tmp = tmp * (*this);

    return tmp;
}

template <typename T>
bool BigGuys<T>::operator> (BigGuys<T> const &cmp) const {
    if (len > cmp.len)
        return true;
    else if (len < cmp.len)
        return false;

    for (size_t i = 0; i < len; i--) {
        if (guy[i] > cmp[i])
            return true;
        else if (guy[i] < cmp[i])
            return false;
    }
}

template <typename T>
T& BigGuys<T>::operator[] (size_t i) const {
    if (i >= len) {
        std::cout << "Your index - " << i << std::endl;
        throw std::runtime_error("Invalid index(too big)");
    }

    return guy[i];
}

template<typename T>
std::istream& operator>> (std::istream& in, BigGuys<T>& foo) {
    string tmp;
    std::getline(std::cin, tmp);
    CHECK(tmp.c_str());
    BigGuys<T> temp(tmp);

    foo = temp;

    return in;
}

template<typename T>
std::ostream& operator<< (std::ostream& out, const BigGuys<T> &foo) {

    size_t i = foo.get_len() - 1;

    if (foo.get_len() == 0) {
        out << std::endl;
        return out;
    }

    out.unsetf(std::ios::dec);
    out.setf(std::ios::hex | std::ios::uppercase);


    std::cout << foo[i--];
    if (foo.get_len() != 1) {
        while(i > 0) {
            out << std::setfill('0') << std::setw(SYM_GROUP) << foo[i--];
        }
        out << std::setfill('0') << std::setw(SYM_GROUP) << foo[i];
    }
    out << std::endl;

    out.unsetf(std::ios::hex | std::ios::uppercase);
    out.setf(std::ios::dec);

    return out;
}

template<typename T>
BigGuys<T>::~BigGuys() {
    delete[] guy;
}

template<typename T>
size_t BigGuys<T>::get_cap() const {
    return cap;
}

template<typename T>
size_t BigGuys<T>::get_len() const {
    return len;
}
