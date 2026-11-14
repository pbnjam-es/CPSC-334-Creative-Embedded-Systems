(
s.waitForBoot {
    // ---- tuning knobs ----
    ~base        = 55;    // A1, the soil's root
    ~attackTime  = 5;    // seconds for a deposit to fade in (use 5 while testing)
    ~decompTime  = 30;   // seconds to break down to residue (use 30 while testing)
    ~maxPartials = 24;

    ~partials = List[];
    ~revBus   = Bus.audio(s, 2);
    ~srcGroup = Group.new;
    ~fxGroup  = Group.after(~srcGroup);

    SynthDef(\soil, { |out=0, freq=55, amp=0.15, cutoff=600|
        var sig = SinOsc.ar(freq * [1, 1.003])
                + (LFTri.ar(freq * 2 * [0.998, 1.002]) * 0.25);
        sig = LPF.ar(sig, cutoff.lag(5)) * amp.lag(5);
        Out.ar(out, sig);
    }).add;

    SynthDef(\partial, { |out=0, freq=220, peak=0.05, residue=0.3,
                          atk=40, decomp=240, pan=0, gate=1, rel=20|
        var amp, bright, sig;
        // fade in, then decay to a lasting residue (fraction of peak)
        amp = EnvGen.kr(Env([0, 1, residue], [atk, decomp], [2, -3])) * peak;
        amp = amp * EnvGen.kr(Env.asr(0.01, 1, rel), gate, doneAction: 2);
        // decomposition = losing brightness
        bright = EnvGen.kr(Env([freq * 8, freq * 1.5], [atk + decomp], \exp));
        sig = SinOsc.ar(freq * LFNoise1.kr(0.1).range(0.997, 1.003))
            + (Saw.ar(freq) * 0.15);
        sig = LPF.ar(sig, bright);
        Out.ar(out, Pan2.ar(sig * amp, pan));
    }).add;

    SynthDef(\reverb, { |in, out=0, mix=0.45, room=0.9|
        var sig = In.ar(in, 2);
        Out.ar(out, FreeVerb2.ar(sig[0], sig[1], mix, room, 0.5));
    }).add;

    s.sync;

    Synth(\reverb, [\in, ~revBus], ~fxGroup);
    ~soil = Synth(\soil, [\out, ~revBus, \freq, ~base], ~srcGroup);

    ~addDeposit = { |ms|
        var w = ms.clip(200, 5000).linlin(200, 5000, 0, 1);   // 0 = light, 1 = heavy
        var h = if(w > 0.5) { [2, 3, 4, 5].choose } { [6, 7, 8, 9, 10, 12].choose };
        var syn = Synth(\partial, [
            \out, ~revBus, \freq, ~base * h,
            \peak, w.linlin(0, 1, 0.04, 0.1) / h.sqrt,
            \atk, ~attackTime, \decomp, ~decompTime,
            \pan, rrand(-0.6, 0.6)
        ], ~srcGroup);
        ~partials.add(syn);
        if(~partials.size > ~maxPartials) { ~partials.removeAt(0).set(\gate, 0) };
        ("deposit " ++ ms ++ " ms -> harmonic " ++ h).postln;
    };

    ~harvest = {
        ~partials.do(_.set(\gate, 0, \rel, 30));
        ~partials = List[];
        "harvest".postln;
    };

    OSCdef(\deposit, { |msg| ~addDeposit.(msg[1]) }, '/compost/deposit');
    OSCdef(\harvest, { ~harvest.() }, '/compost/harvest');

    "compost ready".postln;
};
)