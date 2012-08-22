/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
 *
 * gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:gibbon-m-e
 * @short_description: Abstraction for a match equity table!
 *
 * Since: 0.2.0
 *
 * Abstraction for a match equity table in Gibbon.
 */

#include <math.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-met.h"
#include "gibbon-position.h"

static gfloat rockwell_kazaross_pre[25][25] = {
        /* 1-away.  */
        {
                0.500000f,
                0.676888f,
                0.751179f,
                0.813772f,
                0.841941f,
                0.886867f,
                0.907188f,
                0.932313f,
                0.943975f,
                0.959275f,
                0.966442f,
                0.975534f,
                0.979845f,
                0.985273f,
                0.987893f,
                0.99114f,
                0.99273f,
                0.99467f,
                0.99563f,
                0.99679f,
                0.99737f,
                0.99807f,
                0.99842f,
                0.99884f,
                0.99905f,
        },
        /* 2-away.  */
        {
                0.323112f,
                0.500000f,
                0.598994f,
                0.668586f,
                0.743447f,
                0.798991f,
                0.842141f,
                0.875198f,
                0.901720f,
                0.923034f,
                0.939311f,
                0.952470f,
                0.962495f,
                0.970701f,
                0.976887f,
                0.98196f,
                0.98580f,
                0.98893f,
                0.99129f,
                0.99322f,
                0.99466f,
                0.99585f,
                0.99675f,
                0.99746f,
                0.99802f,
        },
        /* 3-away.  */
        {
                0.248821f,
                0.401006f,
                0.500000f,
                0.571438f,
                0.647713f,
                0.711608f,
                0.762548f,
                0.804850f,
                0.840196f,
                0.870638f,
                0.894417f,
                0.914831f,
                0.930702f,
                0.944426f,
                0.954931f,
                0.96399f,
                0.97093f,
                0.97687f,
                0.98139f,
                0.98522f,
                0.98814f,
                0.99062f,
                0.99248f,
                0.99407f,
                0.99527f,
        },
        /* 4-away.  */
        {
                0.186228f,
                0.331414f,
                0.428562f,
                0.500000f,
                0.577415f,
                0.643063f,
                0.699664f,
                0.746157f,
                0.788290f,
                0.824059f,
                0.853955f,
                0.879141f,
                0.900233f,
                0.918040f,
                0.932657f,
                0.94495f,
                0.95499f,
                0.96341f,
                0.97021f,
                0.97589f,
                0.98044f,
                0.98422f,
                0.98726f,
                0.98975f,
                0.99174f,
        },
        /* 5-away.  */
        {
                0.158059f,
                0.256553f,
                0.352287f,
                0.422585f,
                0.500000f,
                0.566621f,
                0.626658f,
                0.678181f,
                0.725507f,
                0.767055f,
                0.802732f,
                0.833654f,
                0.859934f,
                0.882866f,
                0.902013f,
                0.91847f,
                0.93223f,
                0.94397f,
                0.95367f,
                0.96189f,
                0.96864f,
                0.97432f,
                0.97896f,
                0.98283f,
                0.98600f,
        },
        /* 6-away.  */
        {
                0.113133f,
                0.201009f,
                0.288392f,
                0.356937f,
                0.433379f,
                0.500000f,
                0.562783f,
                0.616561f,
                0.667856f,
                0.713057f,
                0.753427f,
                0.788634f,
                0.819569f,
                0.846648f,
                0.869999f,
                0.89021f,
                0.90756f,
                0.92246f,
                0.93508f,
                0.94583f,
                0.95488f,
                0.96254f,
                0.96894f,
                0.97432f,
                0.97879f,
        },
        /* 7-away.  */
        {
                0.092812f,
                0.157859f,
                0.237452f,
                0.300336f,
                0.373342f,
                0.437217f,
                0.500000f,
                0.554919f,
                0.608614f,
                0.656283f,
                0.700209f,
                0.739054f,
                0.774121f,
                0.805203f,
                0.832566f,
                0.85659f,
                0.87761f,
                0.89591f,
                0.91171f,
                0.92535f,
                0.93702f,
                0.94703f,
                0.95553f,
                0.96276f,
                0.96887f,
        },
        /* 8-away.  */
        {
                0.067687f,
                0.124802f,
                0.195150f,
                0.253843f,
                0.321819f,
                0.383439f,
                0.445081f,
                0.500000f,
                0.554384f,
                0.603718f,
                0.649899f,
                0.691356f,
                0.729447f,
                0.763593f,
                0.794397f,
                0.82158f,
                0.84578f,
                0.86714f,
                0.88589f,
                0.90230f,
                0.91658f,
                0.92898f,
                0.93968f,
                0.94891f,
                0.95682f,
        },
        /* 9-away.  */
        {
                0.056025f,
                0.098280f,
                0.159804f,
                0.211710f,
                0.274493f,
                0.332144f,
                0.391386f,
                0.445616f,
                0.500000f,
                0.550196f,
                0.597926f,
                0.641481f,
                0.682119f,
                0.718927f,
                0.752814f,
                0.78301f,
                0.81037f,
                0.83483f,
                0.85662f,
                0.87591f,
                0.89294f,
                0.90791f,
                0.92098f,
                0.93240f,
                0.94230f,
        },
        /* 10-away.  */
        {
                0.040725f,
                0.076966f,
                0.129362f,
                0.175941f,
                0.232945f,
                0.286943f,
                0.343717f,
                0.396282f,
                0.449804f,
                0.500000f,
                0.548547f,
                0.593459f,
                0.635880f,
                0.674830f,
                0.711113f,
                0.74371f,
                0.77375f,
                0.80093f,
                0.82543f,
                0.84741f,
                0.86703f,
                0.88448f,
                0.89991f,
                0.91353f,
                0.92550f,
        },
        /* 11-away.  */
        {
                0.033558f,
                0.060689f,
                0.105583f,
                0.146045f,
                0.197268f,
                0.246573f,
                0.299791f,
                0.350101f,
                0.402074f,
                0.451453f,
                0.500000f,
                0.545552f,
                0.589242f,
                0.629736f,
                0.667927f,
                0.70303f,
                0.73530f,
                0.76494f,
                0.79198f,
                0.81648f,
                0.83862f,
                0.85849f,
                0.87629f,
                0.89214f,
                0.90622f,
        },
        /* 12-away.  */
        {
                0.024466f,
                0.047530f,
                0.085169f,
                0.120859f,
                0.166346f,
                0.211366f,
                0.260946f,
                0.308644f,
                0.358519f,
                0.406541f,
                0.454448f,
                0.500000f,
                0.544068f,
                0.585701f,
                0.625259f,
                0.66178f,
                0.69610f,
                0.72778f,
                0.75703f,
                0.78381f,
                0.80826f,
                0.83044f,
                0.85051f,
                0.86856f,
                0.88476f,
        },
        /* 13-away.  */
        {
                0.020155f,
                0.037505f,
                0.069298f,
                0.099767f,
                0.140066f,
                0.180431f,
                0.225879f,
                0.270553f,
                0.317881f,
                0.364120f,
                0.410758f,
                0.455932f,
                0.500000f,
                0.541943f,
                0.582545f,
                0.62036f,
                0.65619f,
                0.68966f,
                0.72081f,
                0.74963f,
                0.77619f,
                0.80054f,
                0.82276f,
                0.84295f,
                0.86123f,
        },
        /* 14-away.  */
        {
                0.014727f,
                0.029299f,
                0.055574f,
                0.081960f,
                0.117134f,
                0.153352f,
                0.194797f,
                0.236407f,
                0.281073f,
                0.325170f,
                0.370264f,
                0.414299f,
                0.458057f,
                0.500000f,
                0.540750f,
                0.57942f,
                0.61634f,
                0.65117f,
                0.68391f,
                0.71448f,
                0.74290f,
                0.76917f,
                0.79339f,
                0.81559f,
                0.83586f,
        },
        /* 15-away.  */
        {
                0.012107f,
                0.023113f,
                0.045069f,
                0.067343f,
                0.097987f,
                0.130001f,
                0.167434f,
                0.205603f,
                0.247186f,
                0.288887f,
                0.332073f,
                0.374741f,
                0.417455f,
                0.459250f,
                0.500000f,
                0.53916f,
                0.57679f,
                0.61261f,
                0.64659f,
                0.67859f,
                0.70862f,
                0.73664f,
                0.76265f,
                0.78669f,
                0.80883f,
        },
        /* 16-away.  */
        {
                0.00886f,
                0.01804f,
                0.03601f,
                0.05505f,
                0.08153f,
                0.10979f,
                0.14341f,
                0.17842f,
                0.21699f,
                0.25629f,
                0.29697f,
                0.33822f,
                0.37964f,
                0.42058f,
                0.46084f,
                0.50000f,
                0.53796f,
                0.57441f,
                0.60929f,
                0.64241f,
                0.67376f,
                0.70323f,
                0.73084f,
                0.75657f,
                0.78046f,
        },
        /* 17-away.  */
        {
                0.00727f,
                0.01420f,
                0.02907f,
                0.04501f,
                0.06777f,
                0.09244f,
                0.12239f,
                0.15422f,
                0.18963f,
                0.22625f,
                0.26470f,
                0.30390f,
                0.34381f,
                0.38366f,
                0.42321f,
                0.46204f,
                0.50000f,
                0.53676f,
                0.57222f,
                0.60618f,
                0.63856f,
                0.66925f,
                0.69822f,
                0.72542f,
                0.75087f,
        },
        /* 18-away.  */
        {
                0.00533f,
                0.01107f,
                0.02313f,
                0.03659f,
                0.05603f,
                0.07754f,
                0.10409f,
                0.13286f,
                0.16517f,
                0.19907f,
                0.23506f,
                0.27222f,
                0.31034f,
                0.34883f,
                0.38739f,
                0.42559f,
                0.46324f,
                0.50000f,
                0.53574f,
                0.57023f,
                0.60336f,
                0.63501f,
                0.66510f,
                0.69356f,
                0.72038f,
        },
        /* 19-away.  */
        {
                0.00437f,
                0.00871f,
                0.01861f,
                0.02979f,
                0.04633f,
                0.06492f,
                0.08829f,
                0.11411f,
                0.14338f,
                0.17457f,
                0.20802f,
                0.24297f,
                0.27919f,
                0.31609f,
                0.35341f,
                0.39071f,
                0.42778f,
                0.46426f,
                0.50000f,
                0.53475f,
                0.56838f,
                0.60073f,
                0.63171f,
                0.66122f,
                0.68921f,
        },
        /* 20-away.  */
        {
                0.00321f,
                0.00678f,
                0.01478f,
                0.02411f,
                0.03811f,
                0.05417f,
                0.07465f,
                0.09770f,
                0.12409f,
                0.15259f,
                0.18352f,
                0.21619f,
                0.25037f,
                0.28552f,
                0.32141f,
                0.35759f,
                0.39382f,
                0.42977f,
                0.46525f,
                0.50000f,
                0.53387f,
                0.56667f,
                0.59830f,
                0.62864f,
                0.65760f,
        },
        /* 21-away.  */
        {
                0.00263f,
                0.00534f,
                0.01186f,
                0.01956f,
                0.03136f,
                0.04512f,
                0.06298f,
                0.08342f,
                0.10706f,
                0.13297f,
                0.16138f,
                0.19174f,
                0.22381f,
                0.25710f,
                0.29138f,
                0.32624f,
                0.36144f,
                0.39664f,
                0.43162f,
                0.46613f,
                0.50000f,
                0.53303f,
                0.56508f,
                0.59603f,
                0.62576f,
        },
        /* 22-away.  */
        {
                0.00193f,
                0.00415f,
                0.00938f,
                0.01578f,
                0.02568f,
                0.03746f,
                0.05297f,
                0.07102f,
                0.09209f,
                0.11552f,
                0.14151f,
                0.16956f,
                0.19946f,
                0.23083f,
                0.26336f,
                0.29677f,
                0.33075f,
                0.36499f,
                0.39927f,
                0.43333f,
                0.46697f,
                0.50000f,
                0.53226f,
                0.56360f,
                0.59391f,
        },
        /* 23-away.  */
        {
                0.00158f,
                0.00325f,
                0.00752f,
                0.01274f,
                0.02104f,
                0.03106f,
                0.04447f,
                0.06032f,
                0.07902f,
                0.10009f,
                0.12371f,
                0.14949f,
                0.17724f,
                0.20661f,
                0.23735f,
                0.26916f,
                0.30178f,
                0.33490f,
                0.36829f,
                0.40170f,
                0.43492f,
                0.46774f,
                0.50000f,
                0.53153f,
                0.56221f,
        },
        /* 24-away.  */
        {
                0.00116f,
                0.00254f,
                0.00593f,
                0.01025f,
                0.01717f,
                0.02568f,
                0.03724f,
                0.05109f,
                0.06760f,
                0.08647f,
                0.10786f,
                0.13144f,
                0.15705f,
                0.18441f,
                0.21331f,
                0.24343f,
                0.27458f,
                0.30644f,
                0.33878f,
                0.37136f,
                0.40397f,
                0.43640f,
                0.46847f,
                0.50000f,
                0.53086f,
        },
        /* 25-away.  */
        {
                0.00095f,
                0.00198f,
                0.00473f,
                0.00826f,
                0.01400f,
                0.02121f,
                0.03113f,
                0.04318f,
                0.05770f,
                0.07450f,
                0.09378f,
                0.11524f,
                0.13877f,
                0.16414f,
                0.19117f,
                0.21954f,
                0.24913f,
                0.27962f,
                0.31079f,
                0.34240f,
                0.37424f,
                0.40609f,
                0.43779f,
                0.46914f,
                0.50000f,
        }
};

static gfloat rockwell_kazaross_post[25] = {
        0.500000f,
        0.487677f,
        0.323112f,
        0.310299f,
        0.190796f,
        0.181162f,
        0.115921f,
        0.109060f,
        0.069730f,
        0.065161f,
        0.042069f,
        0.039060f,
        0.025371f,
        0.023428f,
        0.015304f,
        0.01405f,
        0.00924f,
        0.00842f,
        0.00556f,
        0.00505f,
        0.00336f,
        0.00303f,
        0.00203f,
        0.00182f,
        0.00123f,
};

typedef struct _GibbonMETPrivate GibbonMETPrivate;
struct _GibbonMETPrivate {
        gfloat **pre;
        gfloat *post;
};

#define GIBBON_MET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_MET, GibbonMETPrivate))

G_DEFINE_TYPE (GibbonMET, gibbon_met, G_TYPE_OBJECT)

static void gibbon_met_extend_pre (GibbonMET *self, gsize native);
static void gibbon_met_extend_post (GibbonMET *self, gsize native);

static void 
gibbon_met_init (GibbonMET *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_MET, GibbonMETPrivate);

        self->priv->pre = NULL;
        self->priv->post = NULL;
}

static void
gibbon_met_finalize (GObject *object)
{
        GibbonMET *self = GIBBON_MET (object);
        gsize i;

        if (self->priv->pre) {
                for (i = 0; i < GIBBON_MET_MAX_LENGTH; ++i)
                        g_free (self->priv->pre[i]);
                g_free (self->priv->pre);
        }
        g_free (self->priv->post);

        G_OBJECT_CLASS (gibbon_met_parent_class)->finalize(object);
}

static void
gibbon_met_class_init (GibbonMETClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonMETPrivate));

        object_class->finalize = gibbon_met_finalize;
}

/**
 * gibbon_met_new:
 *
 * Creates a new #GibbonMET.
 *
 * Returns: The newly created #GibbonMET or %NULL in case of failure.
 */
GibbonMET *
gibbon_met_new (void)
{
        GibbonMET *self = g_object_new (GIBBON_TYPE_MET, NULL);
        gsize i, j;

        self->priv->pre = g_malloc (GIBBON_MET_MAX_LENGTH
                                    * sizeof *self->priv->pre);
        for (i = 0; i < GIBBON_MET_MAX_LENGTH; ++i)
                self->priv->pre[i] =
                                g_malloc (GIBBON_MET_MAX_LENGTH
                                          * sizeof **self->priv->pre);

        self->priv->post = g_malloc (GIBBON_MET_MAX_LENGTH
                                     * sizeof *self->priv->post);

        for (i = 0; i < 25; ++i) {
                for (j = 0; j < 25; ++j) {
                        self->priv->pre[i][j] = rockwell_kazaross_pre[i][j];
                }
        }

        gibbon_met_extend_pre (self, 25);

        for (i = 0; i < 25; ++i) {
                self->priv->post[i] = rockwell_kazaross_post[i];
        }

        gibbon_met_extend_post (self, 25);

        return self;
}

/*
 * Extend match equity table to native size using David Montgomery's
 * extension algorithm.  The code is pretty much stolen from gnubg.
 */
static void
gibbon_met_extend_pre (GibbonMET *self, gsize native)
{
        static const gfloat stddevs[] = {
                                0.00f, 1.24f, 1.27f, 1.47f, 1.50f, 1.60f,
                                1.61f, 1.66f, 1.68f, 1.70f, 1.72f, 1.77f };
        gsize i, j;
        gsize max_stddevs = sizeof stddevs / sizeof stddevs[0];
        gfloat max_stddev = stddevs[max_stddevs - 1];
        gint score0, score1;
        gfloat games, sigma, stddev0, stddev1;
        gfloat left, right;
        gfloat int1, int2;
        gfloat sqrt2 = sqrtf (2);

        for (i = native; i < GIBBON_MET_MAX_LENGTH; ++i) {
                score0 = i + 1;
                stddev0 = score0 >= max_stddevs ? max_stddev : stddevs[score0];
                for (j = 0; j <= i; ++j) {
                        score1 = j + 1;
                        games = (score0 + score1) / 2.0f;
                        stddev1 = score1 >= max_stddevs ? max_stddev : stddevs[score1];
                        sigma = sqrtf (stddev0 * stddev0 + stddev1 * stddev1)
                                        * sqrtf (games);
                        if (6.0f * sigma > score0 - score1) {
                                left = (gfloat) (score0 - score1);
                                right = 6.0 * sigma;
                                int1 = (erf ((left / sigma) / sqrt2) + 1.0f) 
                                        / 2.0f;
                                int2 = (erf ((right / sigma) / sqrt2) + 1.0f) 
                                        / 2.0f;
                                self->priv->pre[i][j] = int2 - int1;
                        } else {
                                self->priv->pre[i][j] = 0.0f;
                        }
                }
        }

        for (i = 0; i < GIBBON_MET_MAX_LENGTH; i++ ) {
                for (j = ((i < native) ? native : i + 1);
                     j < GIBBON_MET_MAX_LENGTH;
                     ++j) {
                        self->priv->pre[i][j] = 1.0f - self->priv->pre[j][i];
                }
        }
}


/*
 * Extend post-Crawford match equity table to native size.  This code is
 * also stolen from gnubg.
 */
static void
gibbon_met_extend_post (GibbonMET *self, gsize native)
{
        gsize i;

        /*
         * GNUBG throws away the last record because it could be invalid
         * or not present.  We do not follow that example here, tolerating
         * a tiny difference in evaluation results.
         */
        g_assert (native);
        for (i = native; i < GIBBON_MET_MAX_LENGTH; ++i) {
                self->priv->post[i] = GIBBON_MET_GAMMON_RATE * 0.5f
                        * ((i - 4 >= 0) ? self->priv->post[i - 4] : 1.0f)
                        + (1.0f - GIBBON_MET_GAMMON_RATE) * 0.5f
                        * ((i - 2 >= 0) ? self->priv->post[i - 2] : 1.0f);

                g_assert (self->priv->post[i] >= 0.0f
                          && self->priv->post[i] <= 1.0f);

                /*
                 * Add free drop vigorish at 1-away, 2-away and 1-away, 4-away.
                 */
                if (i == 1) {
                        self->priv->post[i] -= GIBBON_MET_FD2;
                        g_assert (self->priv->post[i] >= 0.0f
                                  && self->priv->post[i] <= 1.0f);
                } else if (i == 3) {
                        self->priv->post[i] -= GIBBON_MET_FD4;
                        g_assert (self->priv->post[i] >= 0.0f
                                  && self->priv->post[i] <= 1.0f);
                }
        }
}

gdouble
gibbon_met_get_match_equity (const GibbonMET *self,
                             gsize match_length, guint cube,
                             guint my_score, guint opp_score)
{
        gint me_away, opp_away;

        me_away = match_length - my_score - cube;
        opp_away = match_length - opp_score;

        if (me_away <= 0)
                return 1.0f;
        if (opp_away <= 0)
                return 0.0f;

        g_return_val_if_fail (match_length <= GIBBON_MET_MAX_LENGTH, 0.5f);

        if (opp_away == 1) {
                return self->priv->post[me_away - 1];
        } else if (me_away == 1) {
                return 1 - self->priv->post[opp_away - 1];
        }

        return self->priv->pre[me_away - 1][opp_away - 1];
}

gdouble
gibbon_met_eq2mwc (const GibbonMET *self, gdouble equity,
                   gsize match_length, guint cube,
                   guint my_score, guint opp_score)
{
        gdouble eq_win, eq_lose;

        g_return_val_if_fail (GIBBON_IS_MET (self), 0.0f);

        eq_win = gibbon_met_get_match_equity (self,
                                              match_length, cube,
                                              my_score, opp_score);
        eq_lose = 1 - gibbon_met_get_match_equity (self,
                                                   match_length, cube,
                                                   opp_score, my_score);

        return 0.5f * (equity * (eq_win - eq_lose) + eq_win + eq_lose);
}

gdouble
gibbon_met_mwc2eq (const GibbonMET *self, gdouble mwc,
                   gsize match_length, guint cube,
                   guint my_score, guint opp_score)
{
        gdouble eq_win, eq_lose;

        g_return_val_if_fail (GIBBON_IS_MET (self), 0.0f);

        eq_win = gibbon_met_get_match_equity (self,
                                              match_length, cube,
                                              my_score, opp_score);
        eq_lose = 1 - gibbon_met_get_match_equity (self,
                                                   match_length, cube,
                                                   opp_score, my_score);

        return (2.0f * mwc - (eq_win + eq_lose)) / (eq_win - eq_lose);
}
