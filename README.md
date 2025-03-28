
Lossy Encoding Detector
=======================

This is a [Vamp audio analysis plugin](https://vamp-plugins.org) that
detects whether a music audio recording has been encoded to a lossy
format such as MP3.

It does so by analysing the audio signal without reference to its
current file or stream format, so it can report whether a signal found
in a lossless file (such as WAV or FLAC) was actually the result of
decoding a previous lossy encoding.

Usage
-----

Because this is a Vamp plugin, it needs to be run in a Vamp plugin
host. For example, if you have this plugin compiled and installed in
the usual place (see "Compiling and Installing" below) and you have
`vamp-simple-host` from the [Vamp Plugin
SDK](https://github.com/vamp-plugins/vamp-plugin-sdk) installed in
your PATH, you might run

```
$ vamp-simple-host vamp-lossy-encoding-detector:lossydetector myfile.wav
```

which may produce output similar to this:

```
vamp-simple-host: Running...
Reading file: "myfile.wav", writing to standard output
Running plugin: "lossydetector"...
Using block size = 512, step size = 256
Plugin accepts 1 -> 1 channel(s)
Sound file has 2 (will mix/augment if necessary)
Output is: "lossy"
 0.000000000, 80.050793651: 1 Lossy
```

(If this is a Unix shell, only the last line is sent to stdout and the
rest to stderr, so you could add `2>/dev/null` to the command line to
see only that last line. Much more sophisticated arrangements
involving many input files can be constructed using [Sonic
Annotator](https://vamp-plugins.org/sonic-annotator/).)

The default output of the detector (an output called `lossy`) returns
only the single estimate shown above; it also has an output called
`cf` (for "classification function") which returns an estimate for
each time step. This usually isn't very illuminating because the
method tends to overconfidence one way or another, but occasionally it
betrays some uncertainty:

```
$ vamp-simple-host vamp-lossy-encoding-detector:lossydetector:cf example.wav 2>/dev/null | head -25 | tail -10
 14.980770602: 6.56757e-07 Original
 15.979228627: 0.999746 Lossy
 16.977686652: 0.998157 Lossy
 17.976144677: 0.996165 Lossy
 18.974602702: 0.999981 Lossy
 19.973060727: 1.17946e-05 Original
 20.971518752: 0.998787 Lossy
 21.969976777: 4.33869e-06 Original
 22.968434802: 0.777278 Lossy
 23.966892827: 0.963398 Lossy
$
```

There is also a "quick" version of the plugin, which only has the
default `lossy` output, which runs much faster by inspecting only a
tiny part of the input audio (one second long, starting from 30s into
the file, or at the end if the file is less than 30s long). It's much
faster and actually still works pretty well. To use that, replace
`lossydetector` with `quicklossydetector`.


Compiling and Installing
------------------------

The build uses the [Meson](https://mesonbuild.com/) build system.

First you need the Vamp plugin SDK code checked out into a
subdirectory of this one called `vamp-plugin-sdk`:

```
$ git clone https://github.com/vamp-plugins/vamp-plugin-sdk
```

(Or for developers, `./repoint install` does this)

Then build using Meson and Ninja:

```
$ meson setup build
$ ninja -C build
$ ninja -C build install
```

How reliable is it?
-------------------

In our tests it's about 98% accurate on previously-unseen files ripped
from CD and encoded either lossily or losslessly. This figure is
probably high enough to be useful but not to be used entirely without
supervision.

It can make mistakes in both directions. The very highest-quality
lossy encodings can still be mistaken for lossless, and some lossless
recordings, such as historical recordings using heavy denoising, can
be perceived as lossy.

Method
------

The method is essentially a quick sketch following Hennequin,
Royo-Letelier, and Moussallam,
[Codec-Independent Lossy Audio Compression Detection](http://romain-hennequin.fr/doc/ICASSP2017_Deezer_quality_estimation.pdf),
ICASSP 2017.

It's a convolutional neural network image classifier trained on images
of spectrograms. Our spectrogram configuration is similar to the paper
but our images are smaller (1s in length) and our model has one fewer
fully-connected layer.

Our model was trained using tracks from a small number of commercial
CDs, uncompressed and compressed in various lossy formats, with some
optional augmentations applied beforehand.

The formats used were MP3 (via LAME) in 128k, 192k, VBR Q2; Ogg with
Q3; Opus VBR; AAC (via FAAC) in default and 320k settings. The
augmentations were (separately) resampling to 48kHz, adjusting the
level by -3dB, and applying 1.1x time-stretch using [Rubber Band](https://breakfastquay.com/rubberband/).

Three clips were taken from different parts of each track and
processed in each way, for a total of about 4300 examples. They were
assigned to training or validation based on the identity of the clip,
avoiding the same clip appearing in different forms in
both. Evaluation was carried out using an entirely separate dataset.

The music used in training (being from CD and all) has something of a
90s bias:

* Natalie Merchant - Ophelia
* Mouse on Mars - Autoditacker
* Pere Ubu - Ray Gun Suitcase
* Pet Shop Boys - Bilingual 
* Bob Dylan - Bringing It All Back Home
* Faye Wong - Faye Wong (1997)
* Throwing Muses - Limbo
* Rutland Boughton - Symphony no 3, Oboe concerto no 1 (Helios)
* ELpH vs Coil - Worship the Glitch (4 tracks only)
* Way Out West - Way Out West (3 tracks only)

Copyright
---------

Written by Chris Cannam in the Centre for Digital Music, Queen Mary
University of London. Copyright 2025 Queen Mary University of London.

This code is freely redistributable under a "new-style BSD" licence -
see the header comments in the source for details.

