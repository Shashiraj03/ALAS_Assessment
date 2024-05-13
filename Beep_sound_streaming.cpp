#include <iostream>
#include <opencv2/opencv.hpp>
#include <sys/time.h>
#include <unistd.h>
#include <alsa/asoundlib.h>

using namespace std;
using namespace cv;
struct _curves_data
{
    cv::Point foci_1;
    cv::Point foci_2;

    cv::Point foci_3;
    cv::Point foci_4;

    cv::Point foci_5;
    cv::Point foci_6;
    double image_height;
    double curve_constant;
};
_curves_data curve_data;


snd_pcm_hw_params_t *param;
snd_pcm_t *handle;
unsigned int sample_rate = 44100;


// function for finding foci points
void forci_points(cv::Point &foci_1, cv::Point &foci_2, cv::Point center, float c, float initial_angle)
{
    foci_1 = cv::Point(center.x + c * std::cos(initial_angle), center.y + c * std::sin(initial_angle));
    foci_2 = cv::Point(center.x - c * std::cos(initial_angle), center.y - c * std::sin(initial_angle));
}


void playSingleBeep()
{
    // Prepare a PCM buffer with a single beep sound
    int beepDurationMilliseconds = 200; // Adjust the duration as needed
    int beepFrequencyHz = 1000;         // Adjust the frequency as needed

    int samples = beepDurationMilliseconds * sample_rate / 1000;
    int16_t *buffer = new int16_t[samples * 2];

    for (int i = 0; i < samples; i++)
    {
        double t = static_cast<double>(i) / sample_rate;
        double sine = sin(2.0 * M_PI * beepFrequencyHz * t);
        buffer[i * 2] = static_cast<int16_t>(sine * INT16_MAX);
        buffer[i * 2 + 1] = buffer[i * 2];
    }

    // Open the ALSA PCM stream
    if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
    {
        cerr << "Error Opening PCM Device: " << snd_strerror(errno) << endl;
        exit(1);
    }

    // Set Hardware parameters
    snd_pcm_hw_params_alloca(&param);
    snd_pcm_hw_params_any(handle, param );
    snd_pcm_hw_params_set_access(handle, param, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, param, SND_PCM_FORMAT_S16_LE); // 16 bit little Indian Format
    snd_pcm_hw_params_set_channels(handle, param, 2);
    snd_pcm_hw_params_set_rate_near(handle, param, &sample_rate, 0);
    snd_pcm_hw_params(handle, param);

    // Write the buffer to the PCM device
    int err = snd_pcm_writei(handle, buffer, samples);
    if (err < 0)
    {
        cerr << "Playback error: " << snd_strerror(err) << endl;
        exit(1);
    }

    // Close the ALSA PCM stream
    snd_pcm_close(handle);

    // Free the buffer memory
    delete[] buffer;
}



// for finding mouse click and mouse event
void CallBackFunc(int event, int x, int y, int flags, void *userdata)
{
    if (event == cv::EVENT_MOUSEMOVE)
    {

        cv::Point point(x, y);

        // nearest bumber
        if (y >= (curve_data.image_height - (curve_data.image_height / 4.0)))
        {

            double distence = cv::norm(curve_data.foci_1 - point) + cv::norm(curve_data.foci_2 - point);

            if (distence <= curve_data.curve_constant)
            {
            std:
                cout << "distence : " << distence << "\n";
                std::cout << "curver constant = " << curve_data.curve_constant << "\n";
                if (distence >= (curve_data.curve_constant - 10))
                {
                    std::cout << "play song_1...\n";
                    playSingleBeep();
                }
                std::cout << "nearer to bumber\n";
            }
            else
            {
                std::cout << "Above 1 meter\n";
            }
        }
        else if (y >= (curve_data.image_height / 2.0))
        {
            double distence = cv::norm(curve_data.foci_3 - point) + cv::norm(curve_data.foci_4 - point);
            if (distence <= curve_data.curve_constant)
            {
                if (distence >= (curve_data.curve_constant - 10))
                {
                    std::cout << "play song_2...\n";
                }
                std::cout << "above 1 meter\n";
            }
            else
            {
                std::cout << "Above 2 meter\n";
            }
        }
        else
        {
            double distence = cv::norm(curve_data.foci_5 - point) + cv::norm(curve_data.foci_6 - point);
            if ((y >= (curve_data.image_height / 4.0)) && (distence <= curve_data.curve_constant))
            {
                if (distence >= (curve_data.curve_constant - 10))
                {
                    std::cout << "play song_3...\n";
                }
                std::cout << "Above 2 meter\n";
            }
            else
            {
                std::cout << "Above 3 meter\n";
            }
        }
    }
}


// drawing curve or elipse
void curve(cv::Mat &img, cv::Point center, cv::Size axes, double init_angle, double start_angle, double end_angle, cv::Scalar color, int thickness = 1, int type = 8, int shift = 0)
{
    cv::ellipse(img, center, axes, init_angle, start_angle, end_angle, color, thickness, type, shift);
}

int main()
{
    unsigned int width, height;
    VideoCapture cap(0); // open the default camera with V4L2 backend
    Mat frame;

    // width = 720;
    // height = 576;

    if (!cap.isOpened()) // check if we succeeded
        return EXIT_FAILURE;
    width = cap.get(CAP_PROP_FRAME_WIDTH);
    height = cap.get(CAP_PROP_FRAME_HEIGHT);
    // cout << "Current resolution: Width: " << cap.get(CAP_PROP_FRAME_WIDTH) << " Height: " << cap.get(CAP_PROP_FRAME_HEIGHT) << '\n';

    // Create the window
    namedWindow("Live Stream", WINDOW_NORMAL);
    cout << "Note: Click 'Esc' key to exit the window.\n";

    while (1)
    {

        // cap >> frame; // get a new frame from camera
        cap.read(frame);
        if (frame.empty())
        {
            cerr << "Empty frame received from camera!\n";
            return EXIT_FAILURE;
        }

        // bottom curve
        cv::Point center = cv::Point(frame.size[1] / 2.0, frame.size[0]);
        curve(frame, center, cv::Size(frame.size[1], frame.size[0] / 4), 0, 180, 360, cv::Scalar(0, 0, 255), 4, 16, 0);
        float c = std::sqrt((frame.size[1] * frame.size[1]) - ((frame.size[0] / 4) * (frame.size[0] / 4)));
        curve_data.curve_constant = 2 * frame.size[1];
        curve_data.image_height = frame.size[0];
        forci_points(curve_data.foci_1, curve_data.foci_2, center, c, 0);

        // middle curve
        center = cv::Point(frame.size[1] / 2.0, frame.size[0] - frame.size[0] / 4.0);
        curve(frame, center, cv::Size(frame.size[1], frame.size[0] / 4), 0, 180, 360, cv::Scalar(0, 255, 255), 4, 16, 0);
        forci_points(curve_data.foci_3, curve_data.foci_4, center, c, 0);
        // top curve
        center = cv::Point(frame.size[1] / 2.0, frame.size[0] - frame.size[0] / 2.0);
        curve(frame, center, cv::Size(frame.size[1], frame.size[0] / 4), 0, 180, 360, cv::Scalar(0, 255, 0), 4, 16, 0);
        forci_points(curve_data.foci_5, curve_data.foci_6, center, c, 0);

        // Resize the frame to match the specified width and height
        cv::setMouseCallback("Live Stream", CallBackFunc, NULL);
        resize(frame, frame, Size(width, height));

        // Display the Video

        imshow("Live Stream", frame);

        if (waitKey(1) == 27)
            break;
    }

    
    // the camera will be deinitialized automatically in VideoCapture destructor
    destroyWindow("Live Stream");

    

    return 0;
}
