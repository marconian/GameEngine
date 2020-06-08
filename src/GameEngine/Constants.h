﻿#pragma once

constexpr double PI = 3.1415926535897932384626433832795028841971693993751058209749445923078164062;
const double PI_RAD = PI / 180.;

constexpr double G = 6.673e-11; // gravitational constant (m3)
constexpr double c = 2.99792458e8; // speed of light (ms-1)
constexpr double h = 6.62607004e-34; // Planck's constant (m2 kg / s)
constexpr double eV = 1.60218e-19; // electron volt(J)
constexpr double Rh = 2.179e-18; // Rydberg constant for H (J)
constexpr double sigma = 5.670374419e-8; //Stefan–Boltzmann constant (W⋅m−2⋅K−4)
constexpr double kB = 1.38064852e-23; // Boltzmann constant (m2 kg s-2 K-1)
constexpr double Mu = .99999999965; // Molar mass constant (g mol-1)
constexpr double Na = 6.02214086e23; // Avogadro constant (mol-1)
constexpr double Da = 1.66053906660e-27; // Dalton / unified atomic mass unit
constexpr double R = Na * Da; // Dalton / unified atomic mass unit


constexpr double SOLAR_LUMINOSITY = 3.828e26; // (W)

constexpr double SUN_MASS = 1.989e30;
constexpr double SUN_DIAMETER = 1392680000; // m

constexpr double QUADRANT_SIZE = SUN_DIAMETER * 2.; // m
const double SYSTEM_MASS = SUN_MASS * 1.0014;

constexpr double EARTH_MASS = 5.97219e24;
constexpr double EARTH_DIAMETER = 1.6742e7; // m
constexpr double EARTH_SUN_DIST = 1.496e11; // m
constexpr double EARTH_SUN_VELOCITY = 29780; // (m/s)

constexpr double MOON_MASS = 7.34767309e22;
constexpr double MOON_DIAMETER = 3.4742e6; // m
constexpr double MOON_EARTH_DIST = 3.844e8; // m
constexpr double MOON_EARTH_VELOCITY = 284; // (m/s)

constexpr double JUPITER_MASS = 1.899e27;
constexpr double JUPITER_DIAMETER = 1.42984e8; // m
constexpr double JUPITER_SUN_DIST = 7.7841e11; // m
constexpr double JUPITER_SUN_VELOCITY = 13070; // (m/s)

constexpr double PLUTO_SUN_DIST = 5.9068e12; // m

constexpr double S_NORM = 1.e9;
constexpr double S_NORM_INV = 1. / S_NORM;
constexpr double MASS_RADIUS_NORM = 2.24471369068046E-06;
constexpr double MASS_RADIUS_OFFSET = 1130654.3672034;

const int SECTOR_SIZE = SUN_DIAMETER / S_NORM;
const int SECTOR_DIMS = (EARTH_SUN_DIST / S_NORM) * 3 / SECTOR_SIZE;

constexpr double TIME_DELTA = 0;
constexpr double DEFAULT_ZOOM = 2;
constexpr double ROTATION_GAIN = 1;
constexpr double ROTATION_GAIN_MOUSE = 0.004;
constexpr double MOVEMENT_GAIN = 0.1;

constexpr DXGI_FORMAT BACK_BUFFER_FORMAT = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
constexpr DXGI_FORMAT DEPTH_BUFFER_FORMAT = DXGI_FORMAT_D32_FLOAT;

constexpr bool MSAA_ENABLED = true;
constexpr int SAMPLE_COUNT = 4;

constexpr DirectX::XMVECTORF32 SKY_COLOR = { 0.0249f, 0.03059f, 0.05196f, 1.0f };

const std::string ELEMENTAL_SYMBOLS[109]
{
	"H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne", "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar",
	"K", "Ca", "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge", "As", "Se", "Br",
	"Kr", "Rb", "Sr", "Y", "Zr", "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn", "Sb", "Te",
	"I", "Xe", "Cs", "Ba", "La", "Ce", "Pr", "Nd", "Pm", "Sm", "Eu", "Gd", "Tb", "Dy", "Ho", "Er", "Tm",
	"Yb", "Lu", "Hf", "Ta", "W", "Re", "Os", "Ir", "Pt", "Au", "Hg", "Tl", "Pb", "Bi", "Po", "At", "Rn",
	"Fr", "Ra", "Ac", "Th", "Pa", "U", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm", "Md", "No", "Lr",
	"Rf", "Db", "Sg", "Bh", "Hs", "Mt"
};

const UINT ELEMENTAL_GOLDSCHMIDT[109]
{
	0, 0, 3, 3, 3, 0, 0, 3, 3, 0, 3, 3, 3, 3, 3, 1, 3, 0, 3, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 3, 0, 3, 3, 3, 3, 3, 2, 4, 2, 2, 2, 1, 1,
	1, 1, 1, 1, 3, 0, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 4, 4, 4, 4, 4, 4, 3, 4, 3, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
};

const UINT OXYGEN_BOND[109]
{
	1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0
};

const double ELEMENTAL_DENSITY[109] // (g/cm3)
{
	0.09, 0.18, 0.53, 1.85, 2.34, 2.26, 1.25, 1.43, 1.7, 0.9, 0.97, 1.74, 2.7, 2.33, 1.82, 2.07,
	3.21, 1.78, 0.86, 1.55, 2.99, 4.54, 6.11, 7.19, 7.43, 7.87, 8.9, 8.9, 8.96, 7.13, 5.91, 5.32,
	5.72, 4.79, 3.12, 3.75, 1.63, 2.54, 4.47, 6.51, 8.57, 10.22, 11.5, 12.37, 12.41, 12.02, 10.5,
	8.65, 7.31, 7.31, 6.68, 6.24, 4.93, 5.9, 1.87, 3.59, 6.15, 6.77, 6.77, 7.01, 7.3, 7.52, 5.24,
	7.9, 8.23, 8.55, 8.8, 9.07, 9.32, 6.9, 9.84, 13.31, 16.65, 19.35, 21.04, 22.6, 22.4, 21.45,
	19.32, 13.55, 11.85, 11.35, 9.75, 9.3, 0, 9.73, 0, 5.5, 10.07, 11.72, 15.4, 18.95, 20.2, 19.84,
	13.67, 13.5, 14.78, 15.1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const double ELEMENTAL_WEIGHT[109] // (g/cm3)
{
	1.008, 4.003, 6.941, 9.012, 10.811, 12.011, 14.007, 15.999, 18.998, 20.18, 22.99,
	24.305, 26.982, 28.086, 30.974, 32.065, 35.453, 39.948, 39.098, 40.078, 44.956, 47.867,
	50.942, 51.996, 54.938, 55.845, 58.933, 58.693, 63.546, 65.39, 69.723, 72.64, 74.922,
	78.96, 79.904, 83.8, 85.468, 87.62, 88.906, 91.224, 92.906, 95.94, 98, 101.07, 102.906,
	106.42, 107.868, 112.411, 114.818, 118.71, 121.76, 127.6, 126.905, 131.293, 132.906, 137.327,
	138.906, 140.116, 140.908, 144.24, 145, 150.36, 151.964, 157.25, 158.925, 162.5, 164.93,
	167.259, 168.934, 173.04, 174.967, 178.49, 180.948, 183.84, 186.207, 190.23, 192.217, 195.078,
	196.967, 200.59, 204.383, 207.2, 208.98, 209, 210, 222, 223, 226, 227, 232.038, 231.036, 238.029,
	237, 244, 243, 247, 247, 251, 252, 257, 258, 259, 262, 261, 262, 266, 264, 277, 268
};

const double ELEMENTAL_WEIGHT_N[109]
{
	6.87323e5, 0.000272952, 0.000473285, 0.000614499, 0.000737168, 0.000818992,
	0.000955093, 0.001090921, 0.001295413, 0.00137601, 0.001567615, 0.00165728,
	0.001839816, 0.001915095, 0.002112018, 0.00218641, 0.002417427, 0.002723926,
	0.002665968, 0.002732791, 0.003065406, 0.003263898, 0.003473572, 0.003545441,
	0.003746047, 0.003807892, 0.004018453, 0.004002088, 0.004332999, 0.004458735,
	0.004754189, 0.00495309, 0.005108692, 0.00538403, 0.005448398, 0.005714054,
	0.00582779, 0.005974528, 0.006062216, 0.006220273, 0.006334963, 0.006541842,
	0.006682307, 0.00689164, 0.007016831, 0.00725644, 0.007355174, 0.007664947,
	0.007829073, 0.008094456, 0.008302425, 0.008700636, 0.008653247, 0.00895245,
	0.009062436, 0.009363889, 0.009471556, 0.009554062, 0.009608066, 0.009835265,
	0.009887087, 0.010252568, 0.01036194, 0.010722375, 0.010836588, 0.011080356,
	0.01124605, 0.011404857, 0.01151907, 0.011799045, 0.011930441, 0.012170663,
	0.012338266, 0.012535462, 0.01269686, 0.012971176, 0.013106663, 0.013301746,
	0.01343055, 0.013677591, 0.013936224, 0.014128306, 0.014249679, 0.014251042,
	0.014319229, 0.015137471, 0.015205658, 0.015410218, 0.015478405, 0.01582193,
	0.015753607, 0.016230437, 0.016160273, 0.01663758, 0.016569394, 0.016842141,
	0.016842141, 0.017114888, 0.017183075, 0.017524009, 0.017592196, 0.017660383,
	0.017864943, 0.017796756, 0.017864943, 0.01813769, 0.018001317, 0.018887745,
	0.018274064
};

const double ELEMENTAL_ABUNDANCE[109] // https://arxiv.org/ftp/arxiv/papers/0901/0901.1149.pdf
{
	9.21337200218732e-1, 7.76690404279955e-2, 1.74833953352087e-10, 1.92443128509851e-10, 5.91165166010654e-11,
	2.26089231043436e-4, 6.66633059543929e-5, 4.93685803530174e-4, 2.52817443336471e-10, 1.03453904051864e-4,
	1.81437394036249e-8, 3.23883043080305e-5, 2.66024324704794e-8, 3.14449556388646e-5, 2.60993131802576e-9,
	1.3238326323962e-5, 1.6257042065293e-9, 2.91494738772275e-8, 1.18233033202131e-9, 1.89927532058742e-8,
	1.08170647397694e-10, 7.76690404279955e-10, 8.99325731271527e-11, 4.11928918869126e-9, 2.89922490990331e-9,
	2.66653223817572e-5, 7.38956457513317e-10, 1.54080282630436e-8, 1.70117210006257e-10, 4.08784423305239e-10,
	1.15088537638244e-10, 3.61616989846943e-11, 1.91814229397074e-10, 2.12253450562336e-10, 3.36461025335851e-11,
	1.75462852464864e-10, 2.23259185035938e-10, 7.35811961949431e-11, 1.45590144607943e-10, 3.39605520899737e-11,
	2.45270653983144e-10, 8.01846368791047e-11, 0., 5.59720210371789e-11, 1.16346335863799e-10, 4.27651396688558e-11,
	1.53765833074048e-10, 4.93685803530174e-11, 5.59720210371789e-11, 1.13201840299912e-10, 9.84227111496461e-11,
	1.47476841946275e-10, 3.4589451202751e-11, 1.71689457788201e-10, 1.16660785420188e-10, 1.40558951705725e-10,
	1.43703447269611e-10, 3.71050476538602e-11, 5.40853236988471e-11, 2.69483269825069e-10, 0., 8.33291324429911e-11,
	3.09418363486427e-10, 1.13201840299912e-10, 1.99361018750401e-10, 1.27037620781013e-10, 2.86149096313668e-10,
	8.23857837738252e-11, 1.2766651989379e-10, 8.04990864354933e-11, 1.19490831427685e-10, 4.90541307966287e-11,
	6.60344068416156e-11, 4.30795892252445e-11, 1.7420505423931e-10, 2.13825698344279e-10, 2.1131010189317e-10,
	3.9935093661358e-11, 6.13176634957859e-11, 1.44017896826e-10, 5.72298192627335e-11, 1.04711702277419e-10,
	4.33940387816331e-11, 0., 0., 0., 0., 0., 0., 1.10371794292415e-10, 0., 2.80803453855061e-10, 0., 0., 0.,
	0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.
};

const double ELEMENTAL_ABUNDANCE_T[109]
{
	1.75932e-07, 1.75932e-07, 1.75932e-07, 1.93652e-07, 5.94877e-08, 0.227508955, 0.067081917, 0.496785895,
	2.54405e-07, 0.104103541, 1.82577e-05, 0.032591686, 2.67695e-05, 0.031642414, 2.62632e-06, 0.013321456,
	1.63591e-06, 2.93325e-05, 1.18975e-06, 1.9112e-05, 1.0885e-07, 7.81568e-07, 9.04973e-08, 4.14516e-06,
	2.91743e-06, 0.026832767, 7.43597e-07, 1.55048e-05, 1.71185e-07, 4.11351e-07, 1.15811e-07, 3.63888e-08,
	1.93019e-07, 2.13586e-07, 3.38574e-08, 1.76565e-07, 2.24661e-07, 7.40432e-08, 1.46504e-07, 3.41738e-08,
	2.46811e-07, 8.06882e-08, 0, 5.63235e-08, 1.17077e-07, 4.30337e-08, 1.54731e-07, 4.96786e-08, 5.63235e-08,
	1.13913e-07, 9.90408e-08, 1.48403e-07, 3.48067e-08, 1.72768e-07, 1.17393e-07, 1.41442e-07, 1.44606e-07,
	3.7338e-08, 5.4425e-08, 2.71175e-07, 0, 8.38524e-08, 3.11361e-07, 1.13913e-07, 2.00613e-07, 1.27835e-07,
	2.87946e-07, 8.29031e-08, 1.28468e-07, 8.10046e-08, 1.20241e-07, 4.93622e-08, 6.64491e-08, 4.33501e-08,
	1.75299e-07, 2.15168e-07, 2.12637e-07, 4.01859e-08, 6.17027e-08, 1.44922e-07, 5.75892e-08, 1.05369e-07,
	4.36665e-08, 0, 0, 0, 0, 0, 0, 1.11065e-07, 0, 2.82567e-07, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0
};

const DirectX::XMVECTORF32 ATOM_COLORS[109]
{
	{ 0,0,0,0 },
	{ 0,0,0,0 },
	{ 0.470588235294118,0.352941176470588,0.427450980392157,1 },
	{ 0.101960784313725,0.384313725490196,0.305882352941176,1 },
	{ 0,0,0,0 },
	{ 0,0,0,0 },
	{ 0,0,0,0 },
	{ 0,0,0,0 },
	{ 0,0,0,0 },
	{ 0,0,0,0 },
	{ 0.952941176470588,0.952941176470588,0.96078431372549,1 },
	{ 0.745098039215686,0.749019607843137,0.729411764705882,1 },
	{ 0.8,0.490196078431373,0.423529411764706,1 },
	{ 0.635294117647059,0.647058823529412,0.611764705882353,1 },
	{ 0,0,0,0 },
	{ 0.894117647058824,0.913725490196078,0.6,1 },
	{ 0.517647058823529,0.529411764705882,0.313725490196078,1 },
	{ 0,0,0,0 },
	{ 0.925490196078431,0.458823529411765,0.364705882352941,1 },
	{ 0.949019607843137,0.988235294117647,0.996078431372549,1 },
	{ 0.615686274509804,0.6,0.580392156862745,1 },
	{ 0.588235294117647,0.596078431372549,0.552941176470588,1 },
	{ 0.474509803921569,0.501960784313725,0.541176470588235,1 },
	{ 0.384313725490196,0.4,0.513725490196078,1 },
	{ 0.498039215686275,0.509803921568627,0.541176470588235,1 },
	{ 0.698039215686274,0.486274509803922,0.337254901960784,1 },
	{ 0.337254901960784,0.474509803921569,0.698039215686274,1 },
	{ 0.537254901960784,0.552941176470588,0.549019607843137,1 },
	{ 0.988235294117647,0.52156862745098,0.396078431372549,1 },
	{ 0.592156862745098,0.596078431372549,0.615686274509804,1 },
	{ 0.611764705882353,0.654901960784314,0.662745098039216,1 },
	{ 0.752941176470588,0.756862745098039,0.776470588235294,1 },
	{ 0.8,0.670588235294118,0.294117647058824,1 },
	{ 0.584313725490196,0.627450980392157,0.635294117647059,1 },
	{ 1,0.427450980392157,0.0862745098039216,1 },
	{ 0,0,0,0 },
	{ 0.623529411764706,0.517647058823529,0.529411764705882,1 },
	{ 0.305882352941176,0.435294117647059,0.647058823529412,1 },
	{ 0.6,0.607843137254902,0.552941176470588,1 },
	{ 0.407843137254902,0.12156862745098,0.0862745098039216,1 },
	{ 0.705882352941177,0.705882352941177,0.705882352941177,1 },
	{ 0.454901960784314,0.498039215686275,0.576470588235294,1 },
	{ 0.513725490196078,0.513725490196078,0.505882352941176,1 },
	{ 0.282352941176471,0.329411764705882,0.329411764705882,1 },
	{ 0.827450980392157,0.796078431372549,0.788235294117647,1 },
	{ 0.0470588235294118,0.505882352941176,0.541176470588235,1 },
	{ 0.643137254901961,0.654901960784314,0.682352941176471,1 },
	{ 0.913725490196078,0.850980392156863,0.172549019607843,1 },
	{ 0.686274509803922,0.654901960784314,0.611764705882353,1 },
	{ 0.513725490196078,0.517647058823529,0.498039215686275,1 },
	{ 0.450980392156863,0.509803921568627,0.537254901960784,1 },
	{ 0.52156862745098,0.537254901960784,0.541176470588235,1 },
	{ 0.207843137254902,0.215686274509804,0.223529411764706,1 },
	{ 0,0,0,0 },
	{ 0.890196078431372,0.556862745098039,0.341176470588235,1 },
	{ 0.909803921568627,0.803921568627451,0.729411764705882,1 },
	{ 0.43921568627451,0.435294117647059,0.458823529411765,1 },
	{ 0.635294117647059,0.631372549019608,0.615686274509804,1 },
	{ 0.411764705882353,0.411764705882353,0.372549019607843,1 },
	{ 0.541176470588235,0.529411764705882,0.494117647058824,1 },
	{ 0.505882352941176,0.474509803921569,0.462745098039216,1 },
	{ 0.792156862745098,0.701960784313725,0.490196078431373,1 },
	{ 0.725490196078431,0.670588235294118,0.607843137254902,1 },
	{ 0.533333333333333,0.462745098039216,0.525490196078431,1 },
	{ 0.701960784313725,0.709803921568627,0.654901960784314,1 },
	{ 0.298039215686275,0.294117647058824,0.274509803921569,1 },
	{ 0.427450980392157,0.427450980392157,0.466666666666667,1 },
	{ 0.498039215686275,0.501960784313725,0.482352941176471,1 },
	{ 0.466666666666667,0.443137254901961,0.423529411764706,1 },
	{ 0.796078431372549,0.807843137254902,0.827450980392157,1 },
	{ 0.470588235294118,0.470588235294118,0.470588235294118,1 },
	{ 0.466666666666667,0.482352941176471,0.458823529411765,1 },
	{ 0.501960784313725,0.505882352941176,0.486274509803922,1 },
	{ 0.215686274509804,0.211764705882353,0.203921568627451,1 },
	{ 0.415686274509804,0.427450980392157,0.380392156862745,1 },
	{ 0.827450980392157,0.866666666666667,0.874509803921569,1 },
	{ 0.490196078431373,0.501960784313725,0.466666666666667,1 },
	{ 0.843137254901961,0.850980392156863,0.83921568627451,1 },
	{ 0.890196078431372,0.686274509803922,0.207843137254902,1 },
	{ 0.894117647058824,0.372549019607843,0.447058823529412,1 },
	{ 0.674509803921569,0.607843137254902,0.545098039215686,1 },
	{ 0.337254901960784,0.341176470588235,0.349019607843137,1 },
	{ 0.152941176470588,0.749019607843137,0.454901960784314,1 },
	{ 0.933333333333333,0.83921568627451,0.313725490196078,1 },
	{ 0.152941176470588,0.172549019607843,0.156862745098039,1 },
	{ 0,0,0,0 },
	{ 0.776470588235294,0.396078431372549,0.262745098039216,1 },
	{ 0.752941176470588,0.749019607843137,0.254901960784314,1 },
	{ 0.772549019607843,0.619607843137255,0.325490196078431,1 },
	{ 0.682352941176471,0.47843137254902,0.294117647058824,1 },
	{ 0.00392156862745098,0.513725490196078,0.364705882352941,1 },
	{ 0.870588235294118,0.850980392156863,0.27843137254902,1 },
	{ 0.243137254901961,0.270588235294118,0.23921568627451,1 },
	{ 0.254901960784314,0.235294117647059,0.215686274509804,1 },
	{ 0.701960784313725,0.666666666666667,0.631372549019608,1 },
	{ 0.596078431372549,0.568627450980392,0.458823529411765,1 },
	{ 0.498039215686275,0.498039215686275,0.498039215686275,1 },
	{ 0.498039215686275,0.498039215686275,0.498039215686275,1 },
	{ 0.498039215686275,0.498039215686275,0.498039215686275,1 },
	{ 0.498039215686275,0.498039215686275,0.498039215686275,1 },
	{ 0.498039215686275,0.498039215686275,0.498039215686275,1 },
	{ 0.498039215686275,0.498039215686275,0.498039215686275,1 },
	{ 0.498039215686275,0.498039215686275,0.498039215686275,1 },
	{ 0.498039215686275,0.498039215686275,0.498039215686275,1 },
	{ 0.498039215686275,0.498039215686275,0.498039215686275,1 },
	{ 0.498039215686275,0.498039215686275,0.498039215686275,1 },
	{ 0.498039215686275,0.498039215686275,0.498039215686275,1 },
	{ 0.498039215686275,0.498039215686275,0.498039215686275,1 },
	{ 0.498039215686275,0.498039215686275,0.498039215686275,1 }
};