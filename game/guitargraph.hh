#pragma once

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include "animvalue.hh"
#include "engine.hh"
#include "joystick.hh"
#include "surface.hh"
#include "opengl_text.hh"

class Song;

struct Chord {
	double begin, end;
	bool fret[5];
	Duration const* dur[5];
	int polyphony;
	bool tappable;
	int status; // Guitar: 0 = not played, 10 = tapped, 20 = picked, 30 = released, drums: number of pads hit
	int score;
	Chord(): begin(), end(), polyphony(), tappable(), status(), score() {
		std::fill(fret, fret + 5, false);
		std::fill(dur, dur + 5, static_cast<Duration const*>(NULL));
	}
	bool matches(bool const* fretPressed) {
		if (polyphony == 1) {
			bool shadowed = true;
			for (int i = 0; i < 5; ++i) {
				if (fret[i]) shadowed = false;
				if (!shadowed && fret[i] != fretPressed[i]) return false;
			}
			return true;
		}
		return std::equal(fret, fret + 5, fretPressed);
	}
};

static inline bool operator==(Chord const& a, Chord const& b) {
	return std::equal(a.fret, a.fret + 5, b.fret);
}

/// handles drawing of notes and waves
class GuitarGraph {
  public:
	/// constructor
	GuitarGraph(Audio& audio, Song const& song, bool drums, unsigned num);
	/** draws GuitarGraph
	 * @param time at which time to draw
	 */
	void draw(double time);
	void engine();
	void position(double cx, double width) { m_cx.setTarget(cx); m_width.setTarget(width); }
	double dead(double time) const { return time > -0.5 && m_dead > 50; }
	unsigned stream() const { return m_stream; }
	double correctness() const { return m_correctness.get(); }
  private:
	void fail(double time, int fret);
	Audio& m_audio;
	std::vector<Sample> m_samples;
	input::InputDev m_input;
	Song const& m_song;
	Surface m_button;
	Surface m_tap;
	AnimValue m_hit[6];
	boost::ptr_vector<Texture> m_necks;
	bool m_drums;
	AnimValue m_cx, m_width;
	unsigned m_track;
	std::size_t m_stream;
	std::vector<Track const*> m_tracks;
	void drumHit(double time, int pad);
	void guitarPlay(double time, input::Event const& ev);
	enum Difficulty {
		DIFFICULTY_SUPAEASY,
		DIFFICULTY_EASY,
		DIFFICULTY_MEDIUM,
		DIFFICULTY_AMAZING,
		DIFFICULTYCOUNT
	} m_level;
	struct Event {
		double time;
		AnimValue glow;
		int type; // 0 = miss (pick), 1 = tap, 2 = pick
		int fret;
		Event(double t, int ty, int f = -1): time(t), glow(0.0, 0.5), type(ty), fret(f) { if (type > 0) glow.setValue(1.0); }
	};
	typedef std::vector<Event> Events;
	Events m_events;
	int m_dead;
	glutil::Color const& color(int fret) const;
	void drawBar(double time, float h);
	void difficultyAuto();
	bool difficulty(Difficulty level);
	SvgTxtTheme m_text;
	void updateChords();
	typedef std::vector<Chord> Chords;
	Chords m_chords;
	Chords::iterator m_chordIt;
	typedef std::map<Duration const*, unsigned> NoteStatus; // Note in song to m_events[unsigned - 1] or 0 for not played
	NoteStatus m_notes;
	AnimValue m_correctness;
	int m_score;
};
