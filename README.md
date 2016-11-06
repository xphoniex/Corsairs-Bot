# Corsairs Bot

Corsairs is a game recently introduced in Telegram's platform. You can play it on Android, iOS or install the PC version and the program will give you a link to an HTML5 game. (We use latter here)

This is by no means a complete out-of-box solution to beat the game, just something I did for fun and invite others to improve upon.

The bot fails to work in later level because the DirectX can't keep up with the increased bullet speed. A test shows DirectX is capable of taking upto 25fps which is not enough beyond level 6. There seems to be a solution*. 

# How To Use It

Set the resolution to *1600x900*, everything else must be according to the video.
When you're only one mouse-click away from the game running, press `Num Lock` key, and when the level is about to finish, press `R` key. Repeat for next level.

# How Does It Work

Once we capture each frame, we go through each pixel and check if it has an rgb value of (77,77,77) which is the color found mostly on bullets.
Then we run a scrolling window in the game area and check whether it passes BALLPIXELS threshold. We're almost done, all we need to do now is to calibrate.

My measurements show the spaceship speed to be at 99°/sec and bullet speed to be at 395 pixel/sec.
We use a thread (updateDegree) to keep track of spaceship's current degree which naturally sometimes is out of sync with the real position, thus we use an errorMargin**.

Once we have all the data we can easily detect where each bullet will hit the outer circle and when, then we can check whether we'll be safe or not and act accordingly.

# Playing Strategy

Optimal solution would be to win each level as fast as possible. As a result, we'll mostly be playing Clock-wise and run temporarily Counter-clock-wise when facing obstacles. We use an sleep function with a dynamic value each time we're going CCW so we know when to return to playing CW.***

# Room For Improvement

* \* This link (http://stackoverflow.com/a/22219146) suggests there is a way to capture 60fps. I don't have access to Windows 8 and haven't tried it.
* \*\* The spaceship seems to be using a unique color too, we can detect it with a much greater accuracy the same way we detect bullets BUT it's going to increase our workload per frame.
* \*\*\* If there's a bullet to our right and we go CCW, there's no room for the bot to detect the collision. For our strategy we need to check our move with other bullets too.

# Credits

Screenshot are taken using the code provided here (http://stackoverflow.com/a/30138664) by [Simon Mourier - @smourier](https://github.com/smourier).

