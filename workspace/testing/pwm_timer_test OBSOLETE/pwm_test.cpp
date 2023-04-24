#include <iostream>
#include <fstream>
#include <array>

#include <unistd.h>

auto main() -> int {
	std::fstream fout("/sys/class/pwm/pwmchip0/pwm0/duty_cycle");

	while(true) {
		for(auto&& value : std::to_array({1, 500, 999})) {
			fout << value << std::endl;
			usleep(100);
		}
	}
}
