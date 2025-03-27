/*
    Vamp Lossy Encoding Detector
    Chris Cannam, Queen Mary University of London
    Copyright (c) 2025 Queen Mary University of London

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    Except as contained in this notice, the names of the Centre for
    Digital Music; Queen Mary, University of London; and Chris Cannam
    shall not be used in advertising or otherwise to promote the sale,
    use or other dealings in this Software without prior written
    authorization.
*/

#include <vector>
typedef std::vector<float> t_1;
typedef std::vector<t_1> t_2;
typedef std::vector<t_2> t_3;
typedef std::vector<t_3> t_4;
extern t_4 weights_firstConv;
extern t_4 weights_secondConv;
extern t_4 weights_thirdConv;
extern t_4 weights_fourthConv;
extern t_1 biases_firstConv;
extern t_1 biases_secondConv;
extern t_1 biases_thirdConv;
extern t_1 biases_fourthConv;
extern t_2 weights_firstDense;
extern t_2 weights_labeller;
extern t_1 biases_firstDense;
extern t_1 biases_labeller;
