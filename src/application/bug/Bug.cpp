/*
 * Copyright (C) 2018-2023 Heinrich-Heine-Universitaet Duesseldorf,
 * Institute of Computer Science, Department Operating Systems
 * Burak Akguel, Christian Gesse, Fabian Ruhland, Filip Krakowski, Michael Schoettner
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "Bug.h"

#include "lib/util/game/entity/component/LinearMovementComponent.h"
#include "lib/util/game/entity/event/CollisionEvent.h"
#include "lib/util/game/entity/event/TranslationEvent.h"
#include "lib/util/game/GameManager.h"
#include "lib/util/game/Game.h"
#include "EnemyMissile.h"
#include "PlayerMissile.h"
#include "GameOverScreen.h"
#include "Explosion.h"
#include "application/bug/Fleet.h"
#include "application/bug/Ship.h"
#include "lib/util/collection/Array.h"
#include "lib/util/game/Graphics2D.h"
#include "lib/util/game/Scene.h"
#include "lib/util/game/Sprite.h"
#include "lib/util/game/entity/collider/Collider.h"
#include "lib/util/game/entity/collider/RectangleCollider.h"
#include "lib/util/math/Vector2D.h"

Bug::Bug(const Util::Math::Vector2D &position, Fleet &fleet) : Util::Game::Entity(TAG, position, Util::Game::RectangleCollider(position, Util::Game::Collider::STATIC, SIZE_X, SIZE_Y)), fleet(fleet) {
    addComponent(new Util::Game::LinearMovementComponent(*this));
}

void Bug::initialize() {
    animation = Util::Game::SpriteAnimation(Util::Array<Util::Game::Sprite>({
        Util::Game::Sprite("/initrd/bug/bug1.bmp", SIZE_X, SIZE_Y),
        Util::Game::Sprite("/initrd/bug/bug2.bmp", SIZE_X, SIZE_Y)}), 0.5);

    explosion = Explosion(SIZE_Y, 0.5);
}

void Bug::onUpdate(double delta) {
    if (isExploding) {
        explosionTimer += delta;

        if (explosionTimer >= explosion.getAnimationTime()) {
            Util::Game::GameManager::getCurrentScene().removeObject(this);
            fleet.decreaseSize();
            return;
        }

        currentAnimation->update(delta);
        return;
    }

    currentAnimation->update(delta);

    if (fleet.isMovingDown()) {
        translateY(-0.1);
    }

    if (getPosition().getY() < -0.8 + Ship::SIZE_Y) {
        auto &game = Util::Game::GameManager::getGame();
        game.pushScene(new GameOverScreen(false));
        game.switchToNextScene();
    }

    setVelocityX(fleet.getVelocity());

    if (fleet.getRandomNumber() < delta * 0.01) {
        fireMissile();
    }
}

void Bug::onTranslationEvent(Util::Game::TranslationEvent &event) {
    if (isExploding) {
        event.cancel();
        return;
    }

    const auto targetX = event.getTargetPosition().getX();
    const auto maxX = Util::Game::GameManager::getRelativeResolution().getX() - SIZE_X;

    if ((fleet.getVelocity() > 0 && targetX > maxX - SIZE_X) || (fleet.getVelocity() < 0 && targetX < -maxX)) {
        fleet.moveDown();
        fleet.increaseVelocity();
        fleet.changeDirection();
    }
}

void Bug::onCollisionEvent(Util::Game::CollisionEvent &event) {
    if (event.getCollidedWidth().getTag() == PlayerMissile::TAG) {
        explode();
    }
}

void Bug::draw(Util::Game::Graphics2D &graphics) {
    graphics.drawImage(getPosition(), currentAnimation->getCurrentSprite().getImage());
}

void Bug::fireMissile() {
    auto *missile = new EnemyMissile(getPosition() + Util::Math::Vector2D((SIZE_X / 2) - (EnemyMissile::SIZE_X / 2), -SIZE_Y), *this);
    Util::Game::GameManager::getCurrentScene().addObject(missile);
    missile->setVelocityY(-1);
}

void Bug::explode() {
    isExploding = true;
    currentAnimation = &explosion;
}