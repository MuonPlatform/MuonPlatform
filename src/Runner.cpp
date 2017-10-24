#include "Runner.h"

void signalHandler(int sigNum) {
    std::cout << "Interrupt signal (" << sigNum << ") received. Exiting gracefully..." << std::endl;
    sensors.destroy();
    endwin();
    exit(sigNum);
}

int scp(std::string filename) {
    return system(("scp " + IMG_DIR + "/" + filename + " " + USER + "@" + HOST + ":/home/IMG").c_str());
}

bool kbhit(){
    int ch = getch();
    if(ch!=ERR){
        ungetch(ch);
        return true;
    }
    else{
        return false;
    }
}

void* pwm_thread(void* threadid) {
    long tid;
    tid                                                           = (long) threadid;
    std::chrono::time_point<std::chrono::high_resolution_clock> t = std::chrono::high_resolution_clock::now();
    std::time_t timePt                                            = std::chrono::high_resolution_clock::to_time_t(t);
    while (true) {
        sensors.servo.setValGPIO("1");
        usleep(sensors.servo.getPwmTime());
        sensors.servo.setValGPIO("0");
        usleep(20000 - sensors.servo.getPwmTime());
    }

    pthread_exit(NULL);
}

void* inp_thread(void* threadid) {

    while (true) {
        if (joystick.isFound()) {
            if (joystick.sample(&event) && event.isAxis()) {
                if (event.number == 0) {  // L3 HORIZONTAL
                    sensors.servo.setPwmTime(mid_t + (event.value / 65535.0) * (SERVO_PWM_MAX - SERVO_PWM_MIN));
                }
                else {
                    std::cout << "Controller event: " << event.value << std::endl;
                }
            }
        }
        else {
            if(kbhit())
            {
                contFlag = true;
                int ch = getch();
                switch (ch) {
                    case 'a': sensors.servo.incPwmTime(100); break;
                    case 'd': sensors.servo.incPwmTime(-100); break;
                    case 'A': sensors.servo.setPwmTime(SERVO_PWM_MAX); break;
                    case 'D': sensors.servo.setPwmTime(SERVO_PWM_MIN); break;
                    case 'w':
                        sensors.dc_move(DC_FRWD);
                        break;
                    case 's':
                        sensors.dc_move(DC_BACK);
                        break;
                    case 32:
                        sensors.dc_move(DC_STOP);
                        break;
                    case 27:
                        contFlag = false;
                        break;
                }
            }
        }
    }
}

void* test_thread(void* threadid)
{
    //Ranger rFinder = Ranger();

    while(true)
    {
        if(!kbhit())
        {
            int distance = rFinder.getDistanceMM();
            int rear = 9999;
            int left = 0;
            int right = 0;
            if(distance>0 && distance < 560)    //if something infront is within the minimum 90 deg turning distance
            {
		std::cout << "Object detected" << std::endl;
                while(distance < 560 && rear > 15)  //backup until a turn can be made
                {
                    distance = rFinder.getDistanceMM();
                    rear = ultraBack.getCM();
                    std::cout << "Rear: " << rear << std::endl;
		    sensors.dc_move(DC_BACK);
                }

                sensors.dc_move(DC_STOP);
                //usleep(1000000);
                distance = rFinder.getDistanceMM();
                std::cout << "Finished backing up" << std::endl;
		if(distance <560)   //if distance is still too small for a successful simple turn
                {
		    std::cout << "Do something" << std::endl;
                    //figure something out
                }
                else
                {
                    std::cout << "Distance: " << distance << std::endl;
                    /*
		    while(true)
                    {
                         //check left and right sensors
                        //choose left by default, choose right if something's within the turning arc
                        left = ultraBack.getCM(); //replace with actual sensor
                         //   right = ultraRight.getCM();
                        if(left > 10){
                            sensors.servo.setPwmTime(SERVO_PWM_MAX);
                        }
                    }
                   */
                }
                //crash mitigation
            }
            else
            {
                sensors.dc_move(DC_FRWD);
                //business as usual -- pwm to get lowest possible speed
            }
        }
    }
}

int main(int argc, char** argv) {
    signal(SIGINT, signalHandler);
    pthread_t threads[NUM_THREADS];
    int rc, i = 0;
    sensors.init();
    wiringPiSetupGpio();

    //Setup curses
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    //Create our threads
    rc = pthread_create(&threads[0], NULL, pwm_thread, &i);
    rc = pthread_create(&threads[1], NULL, inp_thread, &(++i));
    rc = pthread_create(&threads[2], NULL, test_thread, &(++i));
    pthread_exit(NULL);

    return 0;
}
