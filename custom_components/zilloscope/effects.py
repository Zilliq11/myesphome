import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_NAME, CONF_LAMBDA, CONF_UPDATE_INTERVAL, CONF_SPEED, CONF_WIDTH, CONF_HEIGHT
from esphome.util import Registry
from .types import DisplayBufferRef, DisplayLambdaEffect, DisplayFireEffect, DisplayBubblesEffect, DisplayMatrixEffect

CONF_DISPLAY_LAMBDA = 'display_lambda'
CONF_DISPLAY_FIRE = 'fire'
CONF_BACKGROUND_COLOR = 'background_color'
CONF_BUBBLES_MINSIZE = 'min_bubble_size'
CONF_BUBBLES_MAXSIZE = 'max_bubble_size'
CONF_LINE_MINSIZE = 'min_line_size'
CONF_LINE_MAXSIZE = 'max_line_size'

ADDRESSABLE_EFFECTS = []
EFFECTS_REGISTRY = Registry()


def register_effect(name, effect_type, default_name, schema, *extra_validators):
    schema = cv.Schema(schema).extend({
        cv.Optional(CONF_NAME, default=default_name): cv.string_strict,
    })
    validator = cv.All(schema, *extra_validators)
    return EFFECTS_REGISTRY.register(name, effect_type, validator)

def register_display_effect(name, effect_type, default_name, schema, *extra_validators):
    # addressable effect can be used only in addressable
    ADDRESSABLE_EFFECTS.append(name)
    return register_effect(name, effect_type, default_name, schema, *extra_validators)


@register_display_effect(
    'display_lambda', DisplayLambdaEffect, "Display Lambda", {
        cv.Required(CONF_LAMBDA): cv.lambda_,
        cv.Optional(CONF_UPDATE_INTERVAL, default='0ms'): cv.positive_time_period_milliseconds,
    }
)
def display_lambda_effect_to_code(config, effect_id):
    args = [(DisplayBufferRef, 'it'), (bool, 'initial_run')]
    lambda_ = yield cg.process_lambda(config[CONF_LAMBDA], args, return_type=cg.void)
    var = cg.new_Pvariable(effect_id, config[CONF_NAME], lambda_,
                           config[CONF_UPDATE_INTERVAL])
    yield var

@register_display_effect('display_fire', DisplayFireEffect, "Fire", {
    cv.GenerateID(): cv.declare_id(DisplayFireEffect),
    cv.Optional(CONF_SPEED, default=15): cv.uint32_t,
    cv.Optional(CONF_WIDTH, default=8): cv.uint32_t,
    cv.Optional(CONF_HEIGHT, default=8): cv.uint32_t,
})
def addressable_fire_effect_to_code(config, effect_id):
    var = cg.new_Pvariable(effect_id, config[CONF_NAME])
    cg.add(var.set_speed(config[CONF_SPEED]))
    cg.add(var.set_width(config[CONF_WIDTH]))
    cg.add(var.set_height(config[CONF_HEIGHT]))
    yield var

@register_display_effect('display_bubbles', DisplayBubblesEffect, "Bubbles", {
    cv.GenerateID(): cv.declare_id(DisplayBubblesEffect),

    cv.Optional(CONF_BACKGROUND_COLOR, default=1651345): cv.uint32_t,
    cv.Optional(CONF_BUBBLES_MINSIZE, default=0): cv.uint8_t,
    cv.Optional(CONF_BUBBLES_MAXSIZE, default=16): cv.uint8_t,

    cv.Optional(CONF_SPEED, default=15): cv.uint32_t,
    cv.Optional(CONF_WIDTH, default=8): cv.uint32_t,
    cv.Optional(CONF_HEIGHT, default=8): cv.uint32_t,
})
def addressable_bubbles_effect_to_code(config, effect_id):
    var = cg.new_Pvariable(effect_id, config[CONF_NAME])
    cg.add(var.set_speed(config[CONF_SPEED]))
    cg.add(var.set_width(config[CONF_WIDTH]))
    cg.add(var.set_height(config[CONF_HEIGHT]))
    yield var

@register_display_effect('display_matrix', DisplayMatrixEffect, "Matrix", {
    cv.GenerateID(): cv.declare_id(DisplayMatrixEffect),

    cv.Optional(CONF_BACKGROUND_COLOR, default=000000): cv.uint32_t,
    cv.Optional(CONF_LINE_MINSIZE, default=0): cv.uint8_t,
    cv.Optional(CONF_LINE_MAXSIZE, default=16): cv.uint8_t,

    cv.Optional(CONF_SPEED, default=15): cv.uint32_t,
    cv.Optional(CONF_WIDTH, default=8): cv.uint32_t,
    cv.Optional(CONF_HEIGHT, default=8): cv.uint32_t,
})
def addressable_matrix_effect_to_code(config, effect_id):
    var = cg.new_Pvariable(effect_id, config[CONF_NAME])
    cg.add(var.set_speed(config[CONF_SPEED]))
    cg.add(var.set_width(config[CONF_WIDTH]))
    cg.add(var.set_height(config[CONF_HEIGHT]))
    yield var

def validate_effects(allowed_effects):
    def validator(value):
        value = cv.validate_registry('effect', EFFECTS_REGISTRY)(value)
        errors = []
        names = set()
        for i, x in enumerate(value):
            key = next(it for it in x.keys())
            if key not in allowed_effects:
                errors.append(
                    cv.Invalid("The effect '{}' is not allowed for this "
                               "light type".format(key), [i])
                )
                continue
            name = x[key][CONF_NAME]
            if name in names:
                errors.append(
                    cv.Invalid("Found the effect name '{}' twice. All effects must have "
                               "unique names".format(name), [i])
                )
                continue
            names.add(name)
        if errors:
            raise cv.MultipleInvalid(errors)
        return value

    return validator
