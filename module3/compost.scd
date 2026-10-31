// compost.scd — Compost bin sonification
// Run the big block with Cmd+Enter (Mac) or `pw-jack sclang -D compost.scd` (Pi).
// Test lines at the bottom: Shift+Enter one at a time.

(
s.waitForBoot {

    // ---------- timing (tuned for a demo of a couple of minutes) ----------
    ~base        = 55;    // A1, root of the soil
    ~burialMin   = 3;     // seconds of silence before a deposit surfaces (min)
    ~burialMax   = 10;    //   ...and max (random in between)
    ~attackTime  = 4;     // fade-in once it surfaces
    ~decompTime  = 45;    // time to rot down to residue
    ~maxPartials = 24;
    ~masterVol   = 4;   // overall volume: 1 = normal, 2-3 = louder (limiter prevents clipping)

    ~partials = List[];
    ~revBus   = Bus.audio(s, 2);
    ~srcGroup = Group.new;
    ~fxGroup  = Group.after(~srcGroup);

    // ---------- the soil: constant low drone, darker as the bin fills ----------
    SynthDef(\soil, { |out=0, freq=55, amp=0.15, cutoff=900|
        var sig = SinOsc.ar(freq * [1, 1.003])
                + (LFTri.ar(freq * 2 * [0.998, 1.002]) * 0.25);
        sig = LPF.ar(sig, cutoff.lag(5)) * amp.lag(5);
        Out.ar(out, sig);
    }).add;

    // ---------- one deposit: surfaces, then rots ----------
    // fmIndex: heavy deposits are clangy/metallic, and the clang fades as it decomposes
    // spread:  light deposits are a shimmering cluster of 3 detuned tones
    // wobble/sawAmt: forceful (fast) presses are unstable and gritty
    SynthDef(\partial, { |out=0, freq=220, peak=0.05, residue=0.3, atk=4, decomp=45,
                          pan=0, gate=1, rel=8, sawAmt=0.15,
                          fmIndex=0, spread=0.004, wobble=0.003|
        var amp, bright, idx, mod, sig;
        amp = EnvGen.kr(Env([0, 1, residue], [atk, decomp], [2, -3])) * peak;
        amp = amp * EnvGen.kr(Env.asr(0.01, 1, rel), gate, doneAction: 2);
        bright = EnvGen.kr(Env([freq * 8, freq * 1.5], [atk + decomp], \exp));
        idx = fmIndex * EnvGen.kr(Env([1, 0.05], [atk + decomp], -2));
        mod = SinOsc.ar(freq * 1.414) * freq * idx;
        sig = SinOsc.ar(
            (freq * [1 - spread, 1, 1 + spread])
            * LFNoise1.kr(0.1 ! 3).range(1 - wobble, 1 + wobble)
            + mod
        ).sum / 3;
        sig = sig + (Saw.ar(freq) * sawAmt);
        sig = LPF.ar(sig, bright);
        Out.ar(out, Pan2.ar(sig * amp, pan));
    }).add;

    // ---------- ghost: a heavy deposit's buried thump resurfacing later ----------
    SynthDef(\ghost, { |out=0, freq=45, amp=0.3, pan=0|
        var env  = EnvGen.kr(Env.perc(0.01, 2.5, curve: -6), doneAction: 2);
        var body = SinOsc.ar(freq * EnvGen.kr(Env([2, 1], [0.08], \exp))) * env;
        var dirt = LPF.ar(PinkNoise.ar, 400) * EnvGen.kr(Env.perc(0.005, 0.4)) * 0.5;
        Out.ar(out, Pan2.ar((body + dirt) * amp, pan));
    }).add;

    // ---------- microbes: clicks/crackle whose activity rises after deposits ----------
    // t_kick adds activity; it decays away over `decay` seconds. Kicks accumulate.
    SynthDef(\microbes, { |out=0, t_kick=0, amp=0.3, decay=40, fill=0|
        var act, dens, clicks, fizz, sig;
        act    = Decay.kr(t_kick, decay).clip(0, 2);
        dens   = act.linexp(0, 2, 0.3, 60);                      // events per second
        clicks = Ringz.ar(Dust.ar(dens ! 2) * 0.1,
                          LFNoise1.kr(0.5 ! 2).exprange(900, 4000), 0.015);
        fizz   = BPF.ar(Dust2.ar(dens * 4 ! 2),
                        LFNoise1.kr(0.2).exprange(2000, 6000), 0.3) * act.sqrt * 0.3;
        sig    = (clicks + fizz) * amp;
        sig    = LPF.ar(sig, fill.lag(3).linexp(0, 1, 8000, 2500)); // deeper = muffled
        Out.ar(out, sig);
    }).add;

    // reverb + master volume + limiter (everything passes through here)
    SynthDef(\reverb, { |in, out=0, mix=0.45, room=0.9, amp=1|
        var sig = In.ar(in, 2);
        sig = FreeVerb2.ar(sig[0], sig[1], mix, room, 0.5) * amp.lag(0.5);
        Out.ar(out, Limiter.ar(sig, 0.95));   // stops clipping when turned up
    }).add;

	// ---------- breath: a weightless deposit (a tissue, or nothing at all) ----------
    SynthDef(\breath, { |out=0, amp=0.05, atk=3, dur=12, freq=1200, pan=0|
        var env = EnvGen.kr(Env([0, 1, 0], [atk, dur], [2, -3]), doneAction: 2);
        var sig = BPF.ar(PinkNoise.ar, freq * LFNoise1.kr(0.3).range(0.8, 1.25), 0.15) * 3;
        Out.ar(out, Pan2.ar(sig * env * amp, pan));
    }).add;

    s.sync;

    ~master   = Synth(\reverb, [\in, ~revBus, \amp, ~masterVol], ~fxGroup);
    ~soil     = Synth(\soil,     [\out, ~revBus, \freq, ~base], ~srcGroup);
    ~microbes = Synth(\microbes, [\out, ~revBus], ~srcGroup);

    // ---------- a deposit arrives from the bin ----------
    ~addDeposit = { |ms, thud = 0, depth = 1, push = 0, rise = 500, fill = 0|
        var t      = thud.clip(0, 2000) / 2000;                       // impact 0..1
        var w      = (t * 0.6) + (push * 0.4);                         // weight 0..1
        var force  = rise.clip(100, 1500).linlin(100, 1500, 1, 0);     // fast press = forceful
        var burial = rrand(~burialMin, ~burialMax);

        ("buried for " ++ burial.round(0.1) ++ "s  (weight " ++ w.round(0.01)
            ++ ", force " ++ force.round(0.01) ++ ")").postln;

        fork {
            var h, syn;
            burial.wait;                                               // the unseen gap

            if(w < 0.05) {
                // nothing / almost nothing: a breath of air, no tone
                Synth(\breath, [\out, ~revBus, \freq, exprand(600, 2500),
                                \pan, rrand(-0.6, 0.6)], ~srcGroup);
                ~microbes.set(\t_kick, 0.1);
                "  surfaced: breath (nothing, or almost nothing)".postln;
            } {
                h = if(w > 0.5) { [2, 3, 4, 5].choose } { [6, 7, 8, 9, 10, 12].choose };
                syn = Synth(\partial, [
                    \out,     ~revBus,
                    \freq,    ~base * h,
                    \peak,    w.linlin(0, 1, 0.06, 0.1) * depth.linlin(0.3, 1, 0.6, 1) / h.sqrt,
                    \fmIndex, w.linlin(0, 1, 0, 3),
                    \spread,  w.linlin(0, 1, 0.012, 0.001),
                    \wobble,  force.linlin(0, 1, 0.002, 0.01),
                    \sawAmt,  force.linlin(0, 1, 0.03, 0.3),
                    \atk,     ~attackTime,
                    \decomp,  ~decompTime * ms.clip(300, 5000).linlin(300, 5000, 0.7, 1.5),
                    \pan,     rrand(-0.6, 0.6)
                ], ~srcGroup);
                ~partials.add(syn);
                if(~partials.size > ~maxPartials) { ~partials.removeAt(0).set(\gate, 0) };

                ~microbes.set(\t_kick, 0.3 + w);
                ("  surfaced: harmonic " ++ h).postln;

                if((push > 0.4) or: { t > 0.5 }) {
                    (~decompTime * 0.5).wait;
                    Synth(\ghost, [\out, ~revBus, \amp, 0.25 * w, \pan, rrand(-0.5, 0.5)], ~srcGroup);
                    (~decompTime * 0.4).wait;
                    Synth(\ghost, [\out, ~revBus, \freq, 38, \amp, 0.1 * w, \pan, rrand(-0.5, 0.5)], ~srcGroup);
                };
            };
        };
	};

    // ---------- the bin is emptied ----------
    ~harvest = {
        ~partials.do(_.set(\gate, 0, \rel, 8));
        ~partials = List[];
        ~microbes.set(\t_kick, 2);                                     // soil turned over
        "harvest".postln;
    };

    // ---------- messages from pedal_listener.py ----------
    OSCdef(\deposit, { |m| ~addDeposit.(m[1], m[2], m[3], m[4], m[5], m[6]) }, '/compost/deposit');
    OSCdef(\fill, { |m|
        ~soil.set(\cutoff, m[1].linexp(0, 1, 900, 250));
        ~microbes.set(\fill, m[1]);
    }, '/compost/fill');
    OSCdef(\harvest, { ~harvest.() }, '/compost/harvest');

    "compost ready".postln;
};
)


// ================= test lines (Shift+Enter one at a time) =================
// Wrapped in a comment so they DON'T run when the Pi starts this file on boot.
// In the Mac IDE you can still put the cursor on a line and press Shift+Enter.
/*

// args: lid ms, thud, depth, push, rise ms, fill
~addDeposit.(800, 20, 0.6, 0, 1200, 0);         // tissue, gentle press: light shimmer
~addDeposit.(1500, 600, 0.9, 0.2, 500, 0.3);     // medium
~addDeposit.(2500, 1800, 1, 0.9, 120, 0.6);      // heavy stomp: low clang + ghost thumps later
~addDeposit.(800, 0, 0.6, 0, 1200, 0);
5.do { ~addDeposit.(rrand(400, 4000), rrand(0, 2000), rrand(0.4, 1.0), 1.0.rand, rrand(100, 1500), 0.3) };
~harvest.();

// live tweaks
~master.set(\amp, 4);          // overall volume up (1 = normal)
~microbes.set(\amp, 0.5);        // louder microbes
~microbes.set(\t_kick, 1);       // poke the pile
~soil.set(\amp, 0.2);
~burialMin = 1; ~burialMax = 4;  // shorter delay
*/