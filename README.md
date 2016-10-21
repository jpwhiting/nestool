NESTool aims to be a better NES Screen Tool.

Before creating this new tool I did try to build NES Screen Tool
with borland C++ builder, but was unable to get the project open.
I believe it will only build with an older version so instead I
decided to rewrite the parts that needed to be rewritten in order
to improve it. I chose to use the Qt framework and C++ for 2 reasons:
 - It is cross platform (should build on windows, linux, and osX).
 - I have lots of experience with Qt/C++

My aim in writing this tool is to make it easier to work with
multiple nametables. The NES Screen Tool 2.04 I used in the past
has a feature to "Remove doubles" and "Remove unused" but only takes
the currently loaded nametable into account when figuring out which
tiles to remove and adjusts only the current nametable. When using the
same tile set for an entire game with multiple nametables it
has no way to take all the nametables into account and make the 
necessary adjustments to all of them. The entire reason I created this
tool is to fix that missing feature.

I would also like to add the ability to import image files directly
creating both the tileset and the nametable(s) required to display the
image given. Hopefully I'll be able to implement that sooner than later.

At any rate I hope you find this tool useful and appreciate any feedback
good or bad about things you would like it to be able to do.

thanks,
    Jeremy Whiting <jpwhiting@kde.org>
