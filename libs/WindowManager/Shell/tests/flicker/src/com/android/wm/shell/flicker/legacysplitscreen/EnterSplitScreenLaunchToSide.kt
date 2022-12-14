/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.wm.shell.flicker.legacysplitscreen

import android.platform.test.annotations.Presubmit
import android.view.Surface
import androidx.test.filters.RequiresDevice
import com.android.server.wm.flicker.FlickerParametersRunnerFactory
import com.android.server.wm.flicker.FlickerTestParameter
import com.android.server.wm.flicker.FlickerTestParameterFactory
import com.android.server.wm.flicker.annotation.Group4
import com.android.server.wm.flicker.dsl.FlickerBuilder
import com.android.server.wm.flicker.helpers.launchSplitScreen
import com.android.server.wm.flicker.helpers.reopenAppFromOverview
import com.android.server.wm.flicker.navBarWindowIsVisible
import com.android.server.wm.flicker.statusBarWindowIsVisible
import com.android.server.wm.traces.common.FlickerComponentName
import com.android.wm.shell.flicker.dockedStackDividerBecomesVisible
import com.android.wm.shell.flicker.dockedStackPrimaryBoundsIsVisibleAtEnd
import com.android.wm.shell.flicker.dockedStackSecondaryBoundsIsVisibleAtEnd
import com.android.wm.shell.flicker.helpers.SplitScreenHelper
import org.junit.FixMethodOrder
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.runners.MethodSorters
import org.junit.runners.Parameterized

/**
 * Test open activity to primary split screen and dock secondary activity to side
 * To run this test: `atest WMShellFlickerTests:EnterSplitScreenLaunchToSide`
 */
@RequiresDevice
@RunWith(Parameterized::class)
@Parameterized.UseParametersRunnerFactory(FlickerParametersRunnerFactory::class)
@FixMethodOrder(MethodSorters.NAME_ASCENDING)
@Group4
class EnterSplitScreenLaunchToSide(
    testSpec: FlickerTestParameter
) : LegacySplitScreenTransition(testSpec) {
    override val transition: FlickerBuilder.() -> Unit
        get() = {
            super.transition(this)
            transitions {
                device.launchSplitScreen(wmHelper)
                device.reopenAppFromOverview(wmHelper)
            }
        }

    override val ignoredWindows: List<FlickerComponentName>
        get() = listOf(LAUNCHER_COMPONENT, splitScreenApp.component,
            secondaryApp.component, FlickerComponentName.SPLASH_SCREEN,
            FlickerComponentName.SNAPSHOT)

    @Presubmit
    @Test
    fun dockedStackPrimaryBoundsIsVisibleAtEnd() =
        testSpec.dockedStackPrimaryBoundsIsVisibleAtEnd(testSpec.startRotation,
            splitScreenApp.component)

    @Presubmit
    @Test
    fun dockedStackSecondaryBoundsIsVisibleAtEnd() =
        testSpec.dockedStackSecondaryBoundsIsVisibleAtEnd(testSpec.startRotation,
            secondaryApp.component)

    @Presubmit
    @Test
    fun dockedStackDividerBecomesVisible() = testSpec.dockedStackDividerBecomesVisible()

    @Presubmit
    @Test
    fun appWindowBecomesVisible() {
        testSpec.assertWm {
            // when the app is launched, first the activity becomes visible, then the
            // SnapshotStartingWindow appears and then the app window becomes visible.
            // Because we log WM once per frame, sometimes the activity and the window
            // become visible in the same entry, sometimes not, thus it is not possible to
            // assert the visibility of the activity here
            this.isAppWindowInvisible(secondaryApp.component)
                    .then()
                    // during re-parenting, the window may disappear and reappear from the
                    // trace, this occurs because we log only 1x per frame
                    .notContains(secondaryApp.component, isOptional = true)
                    .then()
                    .isAppWindowVisible(secondaryApp.component)
        }
    }

    @Presubmit
    @Test
    fun navBarWindowIsVisible() = testSpec.navBarWindowIsVisible()

    @Presubmit
    @Test
    fun statusBarWindowIsVisible() = testSpec.statusBarWindowIsVisible()

    @Presubmit
    @Test
    override fun visibleWindowsShownMoreThanOneConsecutiveEntry() =
            super.visibleWindowsShownMoreThanOneConsecutiveEntry()

    companion object {
        @Parameterized.Parameters(name = "{0}")
        @JvmStatic
        fun getParams(): Collection<FlickerTestParameter> {
            return FlickerTestParameterFactory.getInstance().getConfigNonRotationTests(
                repetitions = SplitScreenHelper.TEST_REPETITIONS,
                supportedRotations = listOf(Surface.ROTATION_0) // bugId = 175687842
            )
        }
    }
}
