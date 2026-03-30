#include "creature.h"

#include <math.h>
#include "raymath.h"
#include <assert.h>

/* ── Public API ──────────────────────────────────────────────── */

void CreatureInit(Creature *c, int id, Vector2 pos, const Genome *genome) {
    assert(c != NULL);
    assert(genome != NULL);

    c->id       = id;
    c->position = pos;
    c->velocity = (Vector2){ 0.0f, 0.0f };
    c->age      = 0.0f;

    /* Copy genome and cache physical traits for fast physics access */
    c->genome      = *genome;
    c->size        = genome->size;
    c->speed       = genome->speed;
    c->vision      = genome->vision;
    c->visionAngle = genome->visionAngle;
    c->metabolism  = genome->metabolism;
    /* lifespan is accessed as c->genome.lifespan — not cached separately */

    /* maxEnergy scales with cross-sectional area (size²) relative to reference size */
    float sizeRatio = c->size / CREATURE_SIZE;
    c->maxEnergy    = CREATURE_MAX_ENERGY * sizeRatio * sizeRatio;
    c->energy       = c->maxEnergy;

    /* Random starting facing angle in radians */
    c->facing = (float)GetRandomValue(0, 360) * DEG2RAD;

    c->reproductionCooldown = 0.0f;
    c->alive                = true;

    /* Zero NN state */
    for (int i = 0; i < NN_INPUTS;    i++) c->nnInputs[i]  = 0.0f;
    for (int i = 0; i < NN_HIDDEN_MAX; i++) c->hiddenOut[i] = 0.0f;
    for (int i = 0; i < NN_OUTPUTS;   i++) c->nnOutputs[i] = 0.0f;
}

void CreatureUpdate(Creature *c, float dt, int worldW, int worldH) {
    assert(c != NULL);
    if (!c->alive) return;

    /* Apply NN outputs to physics */
    float thrust = c->nnOutputs[0];  /* [-1, 1]: forward/backward */
    float turn   = c->nnOutputs[1];  /* [-1, 1]: left/right */

    /* Turn: up to ~3 rad/s */
    c->facing += turn * 3.0f * dt;

    /* Thrust: accelerate in facing direction */
    c->velocity.x += cosf(c->facing) * thrust * c->speed * dt * 3.0f;
    c->velocity.y += sinf(c->facing) * thrust * c->speed * dt * 3.0f;

    /* Clamp velocity magnitude to max speed */
    float velLen = Vector2Length(c->velocity);
    if (velLen > c->speed) {
        c->velocity = Vector2Scale(Vector2Normalize(c->velocity), c->speed);
    }

    /* Move */
    c->position.x += c->velocity.x * dt;
    c->position.y += c->velocity.y * dt;

    /* Toroidal wrap via modulo — adding the dimension before fmodf handles negatives */
    float fw = (float)worldW;
    float fh = (float)worldH;
    c->position.x = fmodf(c->position.x + fw, fw);
    c->position.y = fmodf(c->position.y + fh, fh);

    /* Tick down reproduction cooldown */
    if (c->reproductionCooldown > 0.0f) {
        c->reproductionCooldown -= dt;
        if (c->reproductionCooldown < 0.0f) c->reproductionCooldown = 0.0f;
    }

    /* Weight factor: energy costs scale with cross-sectional area (size²) */
    float sizeRatio = c->size / CREATURE_SIZE;
    float sizeW     = sizeRatio * sizeRatio;

    /* Base metabolism and movement — both scale with body weight */
    float moveCost = 0.03f * Vector2Length(c->velocity);
    float drain    = (c->metabolism + moveCost) * sizeW;

    /* Vision cost: proportional to sector area (r² × halfAngle = area/1).
       Long narrow and short wide cones of equal area cost the same. */
    drain += c->vision * c->vision * c->visionAngle * VISION_COST_SCALE;

    c->energy -= drain * dt;

    if (c->energy <= 0.0f) {
        c->energy = 0.0f;
        c->alive  = false;
    }

    c->age += dt;

    /* Die of old age — lifespan lives in genome */
    if (c->age > c->genome.lifespan) {
        c->alive = false;
    }
}

void CreatureDraw(const Creature *c) {
    assert(c != NULL);
    if (!c->alive) return;

    /* Compute health ratio [0..1] */
    float t = c->energy / c->maxEnergy;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    /* Lerp dark-orange (starving) -> bright-cyan (healthy) */
    Color bodyColor = {
        (unsigned char)(255 * (1.0f - t) * 0.9f),   /* r: high when starving */
        (unsigned char)(180 * t),                     /* g: grows with health  */
        (unsigned char)(220 * t),                     /* b: cyan tint when healthy */
        220
    };

    /* FOV cone — drawn as a proper circular sector */
    float faceDeg  = c->facing * RAD2DEG;
    float halfDeg  = c->visionAngle * RAD2DEG;
    int   segments = (int)(c->visionAngle * 10.0f) + 4;
    DrawCircleSector(c->position, c->vision,
                     faceDeg - halfDeg, faceDeg + halfDeg,
                     segments, (Color){ 100, 210, 255, 8 });
    DrawCircleSectorLines(c->position, c->vision,
                          faceDeg - halfDeg, faceDeg + halfDeg,
                          segments, (Color){ 100, 210, 255, 30 });

    DrawCircleV(c->position, c->size, bodyColor);
    DrawCircleLinesV(c->position, c->size, (Color){ 255, 255, 255, 60 });

    /* Direction line toward current heading */
    Vector2 dir     = { cosf(c->facing), sinf(c->facing) };
    Vector2 lineEnd = Vector2Add(c->position, Vector2Scale(dir, c->size * 1.8f));
    DrawLineV(c->position, lineEnd, (Color){ 255, 255, 255, 120 });
}
