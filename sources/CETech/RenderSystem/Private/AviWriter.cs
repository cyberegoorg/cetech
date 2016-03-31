using System;
using System.IO;

class AviWriter : IDisposable {
    BinaryWriter writer;
    long header;
    long currentRiff;
    long currentMovie;
    int videoWidth;
    int videoHeight;
    int fps;
    int frameCount;
    int frameSize;
    bool flipVertical;
    bool closed;

    public AviWriter (Stream stream, int width, int height, int fps, bool flipVertical) {
        this.fps = fps;
        this.flipVertical = flipVertical;

        videoWidth = width;
        videoHeight = height;
        frameSize = videoWidth * videoHeight * 3;

        writer = new BinaryWriter(stream);
        StartFile();
    }

    public unsafe void WriteFrame (IntPtr data, int size) {
        var srcPitch = videoWidth * 4;
        var dataPtr = (byte*)data;
        if (flipVertical) {
            dataPtr += srcPitch * (videoHeight - 1);
            srcPitch = -srcPitch;
        }

        // write frame data
        var chunk = OpenChunk(FourCC.UncompressedFrame);
        for (int y = 0; y < videoHeight; y++) {
            var linePtr = dataPtr;
            for (int x = 0; x < videoWidth; x++) {
                writer.Write(*linePtr);
                writer.Write(*(linePtr + 1));
                writer.Write(*(linePtr + 2));
                linePtr += 4;
            }
            dataPtr += srcPitch;
        }
        CloseChunk(chunk);

        frameCount++;
    }

    public void Close () {
        if (closed)
            return;

        CloseChunk(currentMovie);
        WriteIndex();
        CloseChunk(currentRiff);

        // rewrite the header with actual data, like frame count
        writer.BaseStream.Position = header - ChunkHeaderSize;
        WriteHeader();

        // close it all down
        writer.Close();
        closed = true;
    }

    void IDisposable.Dispose () {
        Close();
    }

    void StartFile () {
        // start the top-level AVI header
        currentRiff = OpenRiff(FourCC.Riff, FourCC.Avi);
        WriteHeader();

        // start movie list
        currentMovie = OpenList(FourCC.Movie);
    }

    void WriteHeader () {
        // header list
        header = OpenList(FourCC.HeaderList);

        // write AVIMAINHEADER
        var chunk = OpenChunk(FourCC.AviHeader);
        writer.Write((uint)decimal.Round(1000000m / fps));  // microseconds per frame
        writer.Write((uint)(fps * frameSize));              // max bytes per second
        writer.Write(0);                                    // padding granularity
        writer.Write((uint)StandardFlags);                  // flags
        writer.Write(frameCount);                           // total frames
        writer.Write(0);                                    // initial frames
        writer.Write(1);                                    // stream count
        writer.Write(0);                                    // suggested buffer size
        writer.Write(videoWidth);                           // first video stream width
        writer.Write(videoHeight);                          // first video stream height
        writer.Skip(4 * sizeof(uint));                      // reserved bytes
        CloseChunk(chunk);

        // stream list
        var streamList = OpenList(FourCC.StreamList);

        // write AVISTREAMHEADER
        chunk = OpenChunk(FourCC.StreamHeader);
        writer.Write(FourCC.StreamTypeVideo);       // stream type
        writer.Write(FourCC.CodecUncompressed);     // codec
        writer.Write(0);                            // flags
        writer.Write((ushort)0);                    // priority
        writer.Write((ushort)0);                    // language
        writer.Write(0);                            // initial frames
        writer.Write(1);                            // scale
        writer.Write(fps);                          // rate
        writer.Write(0);                            // start
        writer.Write(frameCount);                   // frame count
        writer.Write(frameSize);                    // suggested buffer size
        writer.Write(0);                            // quality
        writer.Write(0);                            // sample size
        writer.Write((short)0);                     // rectangle left
        writer.Write((short)0);                     // rectangle top
        writer.Write((short)videoWidth);            // rectangle right
        writer.Write((short)videoHeight);           // rectangle bottom
        CloseChunk(chunk);

        // write BITMAPINFOHEADER
        chunk = OpenChunk(FourCC.StreamFormat);
        writer.Write(40);                           // size of structure
        writer.Write(videoWidth);                   // width
        writer.Write(videoHeight);                  // height
        writer.Write((short)1);                     // color planes
        writer.Write((ushort)24);                   // bits per pixel
        writer.Write(FourCC.CodecUncompressed);     // compression
        writer.Write(frameSize);                    // size in bytes of a frame
        writer.Write(0);                            // X pixels per meter
        writer.Write(0);                            // Y pixels per meter
        writer.Write(0);                            // palette colors
        writer.Write(0);                            // important palette colors
        CloseChunk(chunk);

        // close lists
        CloseChunk(streamList);
        CloseChunk(header);
    }

    void WriteIndex () {
        var indexChunk = OpenChunk(FourCC.Index1);
        var offset = 4;
        for (int i = 0; i < frameCount; i++) {
            var chunk = OpenChunk(FourCC.UncompressedFrame);
            writer.Write(offset);
            writer.Write(frameSize);
            offset += frameSize + ChunkHeaderSize;
            CloseChunk(chunk);
        }

        CloseChunk(indexChunk);
    }

    long OpenChunk (FourCC fourCC) {
        writer.Write(fourCC);
        writer.Write(0);    // chunk size is initially unknown
        return writer.BaseStream.Position;
    }

    long OpenList (FourCC fourCC) {
        return OpenRiff(FourCC.List, fourCC);
    }

    long OpenRiff (FourCC listType, FourCC fourCC) {
        var result = OpenChunk(listType);
        writer.Write(fourCC);
        return result;
    }

    void CloseChunk (long offset) {
        var pos = writer.BaseStream.Position;
        var size = pos - offset;
        if (size > int.MaxValue - ChunkHeaderSize)
            throw new InvalidOperationException("Avi chunk is too big.");

        // seek back to the size member and overwrite it
        writer.BaseStream.Position = offset - sizeof(uint);
        writer.Write((uint)size);
        writer.BaseStream.Position = pos;

        // pad to WORD boundaries
        if ((pos & 0x1) != 0)
            writer.Skip(1);
    }

    const int ChunkHeaderSize = 2 * sizeof(uint);
    const MainHeaderFlags StandardFlags = MainHeaderFlags.IsInterleaved | MainHeaderFlags.TrustChunkType | MainHeaderFlags.HasIndex;

    [Flags]
    enum MainHeaderFlags : uint {
        None = 0,
        HasIndex = 0x00010U,
        MustUseIndex = 0x00020U,
        IsInterleaved = 0x00100U,
        TrustChunkType = 0x00800U,
        WasCaptureFile = 0x10000U,
        Copyrighted = 0x200000U
    }

    // helper wrapper around 4CC codes for debugging purposes
    struct FourCC {
        uint value;
        public FourCC (string str) {
            if (str.Length != 4)
                throw new InvalidOperationException("Invalid FourCC code");
            value = str[0] | ((uint)str[1] << 8) | ((uint)str[2] << 16) | ((uint)str[3] << 24);
        }

        public override string ToString () {
            return new string(new[] {
                    (char)(value & 0xff),
                    (char)((value >> 8) & 0xff),
                    (char)((value >> 16) & 0xff),
                    (char)(value >> 24)
                });
        }

        public static implicit operator uint (FourCC fourCC) {
            return fourCC.value;
        }

        // predefined FourCC codes
        public static readonly FourCC Avi = new FourCC("AVI ");
        public static readonly FourCC Riff = new FourCC("RIFF");
        public static readonly FourCC List = new FourCC("LIST");
        public static readonly FourCC HeaderList = new FourCC("hdrl");
        public static readonly FourCC AviHeader = new FourCC("avih");
        public static readonly FourCC StreamList = new FourCC("strl");
        public static readonly FourCC StreamHeader = new FourCC("strh");
        public static readonly FourCC StreamFormat = new FourCC("strf");
        public static readonly FourCC StreamIndex = new FourCC("indx");
        public static readonly FourCC Movie = new FourCC("movi");
        public static readonly FourCC UncompressedFrame = new FourCC("00db");
        public static readonly FourCC StreamTypeVideo = new FourCC("vids");
        public static readonly FourCC CodecUncompressed = new FourCC();
        public static readonly FourCC Index1 = new FourCC("idx1");
    }
}

static class BinaryWriterExtensions {
    static readonly byte[] emptyBytes = new byte[1024];

    public static void Skip (this BinaryWriter writer, int count) {
        while (count > 0) {
            var subset = Math.Min(count, emptyBytes.Length);
            writer.Write(emptyBytes, 0, subset);
            count -= subset;
        }
    }
}
