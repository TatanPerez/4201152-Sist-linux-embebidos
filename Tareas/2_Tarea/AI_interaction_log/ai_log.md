# AI Interaction Log — Embedded Controller Project

This file documents the use of an artificial intelligence assistant (ChatGPT) during the development of a modular project involving sensors, actuators, and a controller. It includes exchanged messages, AI-generated responses, and developer comments explaining why certain questions were asked and how the process evolved.  
[View conversation on ChatGPT](https://chatgpt.com/share/68d6948f-cbe0-800e-b9f0-eb4983675c87)

📅 Date: 2025-09-25

---

## Interaction 1: Explanation of the project structure

**Prompt:**

> I created a project with this structure, which includes some exercises that I need an explanation for to understand how they work.

> (Structure shown: Makefile + sensor/ + actuators/ + controller/ + tests/)

**Assistant’s Response:**

- Explained the purpose of each folder (sensor/, actuators/, controller/, etc.).
- Detailed the relationship between `.h` and `.c` files.
- Described how a closed-loop embedded controller works.
- Offered to create a base template as an example.

**Comment:**

I asked for this explanation because I needed to understand how the different files in the project fit together before starting to code. It helped me visualize the data flow from the sensor to the actuators.

---

## Interaction 2: Developing the assignment exercises

**Prompt:**

> I have an activity to do  
> (Includes the 3 project exercises: modular sensor, polymorphic interface, closed-loop controller)

**Assistant’s Response:**

- Detailed breakdown of the 3 exercises:
  - How to implement `sensor.h` and `sensor.c` with include guards.
  - How to create a polymorphic interface using function pointers.
  - How to implement `ctl.c` with sensor reading, THRESHOLD comparison, and delayed shutdown.
- Provided code examples and data structures.
- Suggested a Makefile with targets: `ctl64`, `ctl32`, `clean`.

**Comment:**

This was the key interaction. It helped me understand how to break down the project into small parts, and especially how to use function pointers in C to simulate polymorphism.

---

## Interaction 3: Confusion with make vs binary execution

**Prompt:**

> So if I do `make`  
> and then `./make`  
> `ctl64`

**Assistant’s Response:**

- Explained the difference between `make` (a build tool) and `./ctl64` (running the compiled binary).
- Clarified the mistake: `./make` should not be used.
- Suggested adding a `run64` target to the Makefile to compile and run in one step.

**Comment:**

I made a mistake thinking `./make` would execute something. This answer helped me clearly differentiate between compiling (`make ctl64`) and running (`./ctl64`), which is essential in C.

---

## Interaction 4: What output should I see in the console?

**Prompt:**

> What would the output be?

**Assistant’s Response:**

- Provided a simulation of the expected console output:
  - Timestamps, sensor values, LED and buzzer states.
- Explained the expected behavior according to the THRESHOLD.
- Included a full example of simulated output.

**Comment:**

I wanted to verify if my controller was working correctly. The output example was helpful for validating the logic of turning actuators on/off at the correct time.

---

## Interaction 5: Request to create ai_log.md

**Prompt:**

> Now create an `ai_log.md` file that:  
> * Contains the messages and responses from this chat as the project was built.  
> * Includes brief comments about why each question was asked and how they were refined.

**Assistant’s Response:**

- Generated this file with chronological content.
- Each interaction includes prompt, response, and explanatory comment.

**Comment:**

This documentation was required as part of the deliverable. It reflects the AI-guided learning process and also helped me review the steps I followed to structure the project correctly.

---
