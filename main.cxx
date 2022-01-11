#include <iostream>
#include <vector>
#include <functional>
#include <list>
#include <arpa/inet.h>
#include <netinet/in.h>

// well, that's a bit ugly
std::ostream& operator << (std::ostream &os, in_addr addr) {
	// we keep addresses in host order, while inet_ntoa expects them in network order.
	os << ::inet_ntoa({htonl(addr.s_addr)});
	return os;
}

template <class What>
struct MatchingState {
	std::function<bool (What&)> matcher;

	template <class Matcher>
	MatchingState(Matcher ma) : matcher(ma) {}
};

template <class What>
class Matching {
	std::list<MatchingState<What>> matchers;
public:
	void AddMatcher(std::function<bool (What&)> m) noexcept {
		matchers.push_back(std::move(MatchingState<What>(m)));
	}
	void MatchAndDump(std::vector<What> &w, std::ostream &os) {
		for (auto &m : matchers) {
			for (auto &val : w) {
				if (m.matcher(val)) {
					os << val << std::endl;
				}
			}
		}
	}
};

int main() {

	std::vector<in_addr> addresses;
	Matching<in_addr>  m;

	/// Code below implies that the machine is Little Endian
	/// Real production code would be way more careful here (or not :)).

	/// This one matches everything, so we receive full array output on a first pass.
	m.AddMatcher([](in_addr a) {
		std::ignore = a;
		return true;
	});
	/// Match those whose first octet is 1
	m.AddMatcher([](in_addr a) {
		return ((a.s_addr >> 24) & 0xff) == 1;
	});
	/// And so on
	m.AddMatcher([](in_addr a) {
		return ((a.s_addr >> 24) & 0xff) == 46 && ((a.s_addr >> 16) & 0xff) == 70;
	});
	m.AddMatcher([](in_addr a) {
		return  ((a.s_addr & 0xff) == 46) ||
			(((a.s_addr >> 8) & 0xff) == 46) ||
			(((a.s_addr >> 16) & 0xff) == 46) ||
			(((a.s_addr >> 24) & 0xff) == 46);
	});

	std::string input;
	while (std::getline(std::cin, input)) {
		addresses.push_back({ntohl(inet_addr(input.c_str()))});
	}
	std::sort(addresses.begin(), addresses.end(),
		[] (const in_addr &a, const in_addr &b) {
			return a.s_addr > b.s_addr;
		});
	m.MatchAndDump(addresses, std::cout);
	return 0;
}
