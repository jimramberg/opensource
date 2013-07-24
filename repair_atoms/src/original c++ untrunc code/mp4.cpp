#include "mp4.h"
#include "atom.h"
#include "file.h"

#include <assert.h>
#include <string>
#include <iostream>

#define __STDC_LIMIT_MACROS 1
#define __STDC_CONSTANT_MACROS 1
extern "C" {
#include <stdint.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

using namespace std;

Mp4::Mp4(): root(NULL) { }

Mp4::~Mp4() {

    delete root;
}

void Mp4::open(string filename) {
    File file;
    bool success = file.open(filename);
    if(!success) throw string("Could not open file: ") + filename;

    root = new Atom;
    while(1) {
        Atom *atom = new Atom;
        atom->parse(file);
        root->children.push_back(atom);
        if(file.atEnd()) break;
    }

    if(root->atomByName("ctts"))
        cerr << "Composition time offset atom found. Out of order samples possible." << endl;

    if(root->atomByName("sdtp"))
        cerr << "Sample dependency flag atom found. I and P frames might need to recover that info." << endl;

    Atom *mvhd = root->atomByName("mvhd");
    if(!mvhd)
        throw string("Missing movie header atom");
    timescale = mvhd->readInt(12);
    duration = mvhd->readInt(16);

    av_register_all();
    context = avformat_alloc_context();
    // Open video file
    //int error = av_open_input_file(&context, filename.c_str(), NULL, 0, NULL);
    int error = avformat_open_input(&context, filename.c_str(), NULL, NULL);

    if(error != 0)
        throw "Could not parse AV file: " + filename;

    //if(av_find_stream_info(context)<0)
    if(avformat_find_stream_info(context, NULL) < 0)
        throw string("Could not find stream info");

    av_dump_format(context, 0, filename.c_str(), 0);

    parseTracks();
}

void Mp4::printMediaInfo() {

}

void Mp4::printAtoms() {
    if(root)
        root->print(0);
}

void Mp4::saveVideo(string filename) {
    /* we save all atom except:
      ctts: composition offset ( we use sample to time)
      cslg: because it is used only when ctts is present
      stps: partial sync, same as sync

      movie is made by ftyp, moov, mdat (we need to know mdat begin, for absolute offsets)
      assumes offsets in stco are absolute and so to find the relative just subtrack mdat->start + 8
*/

    duration = 0;
    for(unsigned int i = 0; i < tracks.size(); i++) {
        Track &track = tracks[i];
        track.writeToAtoms();
        //convert to movie timescale
        int track_duration = (int)(double)track.duration * ((double)timescale / (double)track.timescale);
        if(track_duration > duration) duration = track_duration;

        Atom *tkhd = track.trak->atomByName("tkhd");
        tkhd->writeInt(track_duration, 20); //in movie timescale, not track timescale
    }
    Atom *mvhd = root->atomByName("mvhd");
    mvhd->writeInt(duration, 16);


    Atom *ftyp = root->atomByName("ftyp");
    Atom *moov = root->atomByName("moov");
    Atom *mdat = root->atomByName("mdat");

    moov->prune("ctts");
    moov->prune("cslg");
    moov->prune("stps");

    root->updateLength();

    //fix offsets
    int offset = ftyp->length + moov->length - mdat->start;
    for(unsigned int t = 0; t < tracks.size(); t++) {
        Track &track = tracks[t];
        for(unsigned int i = 0; i < track.offsets.size(); i++)
            track.offsets[i] += offset;

        track.writeToAtoms();  //need to save the offsets back to the atoms
    }

    //save to file
    File file;
    if(!file.create(filename))
        throw "Could not create file for writing: " + filename;

    ftyp->write(file);
    moov->write(file);
    mdat->write(file);
}

void Mp4::analyze() {


    Atom *mdat = root->atomByName("mdat");
    for(unsigned int i = 0; i < tracks.size(); i++) {
        cout << "\n\n Track " << i << endl;
        Track &track = tracks[i];

        cout << "Track codec: " << track.codec.codec << endl;

        cout << "Keyframes  " << track.keyframes.size() << endl;
        for(unsigned int i = 0; i < track.keyframes.size(); i++) {
            int k = track.keyframes[i];
            int offset = track.offsets[k] - (mdat->start + 8);
            int begin =  mdat->readInt(offset);
            int next =  mdat->readInt(offset + 4);
            cout << k << " Size: " << track.sizes[k] << " offset " << track.offsets[k]
                 << "  begin: " << hex << begin << " " << next << dec << endl;
        }

        for(unsigned int i = 0; i < track.offsets.size(); i++) {
            int offset = track.offsets[i] - (mdat->start + 8);
            unsigned char *start = &(mdat->content[offset]);
            int maxlength = mdat->content.size() - offset;

            int begin =  mdat->readInt(offset);
            int next =  mdat->readInt(offset + 4);
            int end = mdat->readInt(offset + track.sizes[i] - 4);
            cout << i << " Size: " << track.sizes[i] << " offset " << track.offsets[i]
                 << "  begin: " << hex << begin << " " << next << " end: " << end << dec << endl;

            bool matches = track.codec.matchSample(start, maxlength);
            int length= track.codec.getLength(start, maxlength);
            if(!matches) {
                cout << "Match failed! Hit enter for next match." << endl;
                getchar();
            }
            //assert(matches);
            cout << "Length: " << length << " true length: " << track.sizes[i] << endl;
            //assert(length == track.sizes[i]);

        }
    }

}

void Mp4::writeTracksToAtoms() {
    for(unsigned int i = 0; i < tracks.size(); i++)
        tracks[i].writeToAtoms();
}

void Mp4::parseTracks() {
    Atom *mdat = root->atomByName("mdat");
    vector<Atom *> traks = root->atomsByName("trak");
    for(unsigned int i = 0; i < traks.size(); i++) {
        Track track;
        track.codec.context = context->streams[i]->codec;
        track.parse(traks[i], mdat);
        tracks.push_back(track);
    }
}

void Mp4::repair(string filename) {


    File file;
    if(!file.open(filename))
        throw "Could not open file: " + filename;

    //find mdat. fails with krois
    //TODO check for multiple mdat
    Atom *mdat = NULL;
    while(1) {

        Atom *atom = new Atom;
        try {
            atom->parseHeader(file);
        } catch(string) {
            throw string("Failed to parse atoms in truncated file");
        }

        if(atom->name != string("mdat")) {
            int pos = file.pos();
            file.seek(pos - 8 + atom->length);
            delete atom;
            continue;
        }

        mdat = atom;
        mdat->content = file.read(file.length() - file.pos());
        break;
    }

    for(unsigned int i = 0; i < tracks.size(); i++)
        tracks[i].clear();

    unsigned long offset = 0;
    while(offset < mdat->content.size()) {
        unsigned char *start = &(mdat->content[offset]);

        long begin =  mdat->readInt(offset);

        //Skip zeros to next 000
        if(begin == 0) {
            offset &= 0xfffff000;
            offset += 0x1000;
            continue;
        }
        long next =  mdat->readInt(offset + 4);

        long maxlength = mdat->content.size() - offset;
        if(maxlength >1000000) maxlength = 1000000;

        cout << "Begin: " << hex << begin << " " << next << dec;
        bool found = false;
        for(unsigned int i = 0; i < tracks.size(); i++) {
            Track &track = tracks[i];
            //sometime audio packets are difficult to match, but if they are the only ones....
            if(tracks.size() > 1 && !track.codec.matchSample(start, maxlength)) continue;
            int length = track.codec.getLength(start, maxlength);
            if(length < -1 || length > 800000) {
                cout << endl << "Invalid length. " << length << ". Wrong match in track: " << i << endl;
                continue;
            }
            if(length == -1 || length == 0) {
                continue;
            }
            if(length >= maxlength)
                continue;
            cout << ": found as " << track.codec.name;
            bool keyframe = track.codec.isKeyframe(start, maxlength);
            if(keyframe)
                track.keyframes.push_back(track.offsets.size());
            track.offsets.push_back(offset);
            track.sizes.push_back(length);
            offset += length;

            found = true;
            break;
        }
        cout << endl;
        if(!found) {
            //this could be a problem for large files
            //assert(mdat->content.size() + 8 == mdat->length);
            mdat->content.resize(offset);
            mdat->length = mdat->content.size() + 8;
            break;
        }

    }

    for(unsigned int i = 0; i < tracks.size(); i++)
        tracks[i].fixTimes();

    Atom *original_mdat = root->atomByName("mdat");
    original_mdat->content.swap(mdat->content);
    original_mdat->start = -8;
}
