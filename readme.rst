This software implements the CU structure reuse method proposed in the paper: D. Schroeder, P. Rehm, and E. Steinbach, "Block structure reuse for multi-rate High Efficiency Video Coding", Proc. IEEE International Conference on Image Processing (ICIP), Sep. 2015, Quebec, Canada. A free copy of this paper can be found at: http://www.lmt.ei.tum.de/forschung/publikationen/dateien/Schroeder2015Blockstructurereusefor.pdf


====================================================
HOW TO
====================================================

A new CLI option is available: --mr-mode

DEFAULT
By default, the multi-rate encoding mode is off.

WRITE MODE (--mr-mode 1):
When mr-mode is 1, a file called analysisData.bin containing the CU structure of the present (reference) encoding is created.

LOAD MODE (--mr-mode 2):
When mr-mode is 2, the (previously created) analysisData.bin file is read and the present (dependent) encoding is shortened according to the method proposed in the paper.

example:

./x265 video.yuv -o bitstream.bin --qp 22 --mr-mode 1

./x265 video.yuv -o bitstream.bin --qp 27 --mr-mode 2

./x265 video.yuv -o bitstream.bin --qp 32 --mr-mode 2

./x265 video.yuv -o bitstream.bin --qp 37 --mr-mode 2



=================
x265 HEVC Encoder
=================

| **Read:** | Online `documentation <http://x265.readthedocs.org/en/default/>`_ | Developer `wiki <http://bitbucket.org/multicoreware/x265/wiki/>`_
| **Download:** | `releases <http://ftp.videolan.org/pub/videolan/x265/>`_ 
| **Interact:** | #x265 on freenode.irc.net | `x265-devel@videolan.org <http://mailman.videolan.org/listinfo/x265-devel>`_ | `Report an issue <https://bitbucket.org/multicoreware/x265/issues?status=new&status=open>`_

`x265 <https://www.videolan.org/developers/x265.html>`_ is an open
source HEVC encoder. See the developer wiki for instructions for
downloading and building the source.

x265 is free to use under the `GNU GPL <http://www.gnu.org/licenses/gpl-2.0.html>`_ 
and is also available under a commercial `license <http://x265.org>`_ 
