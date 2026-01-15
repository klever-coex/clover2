"use client";

import clsx from "clsx";

type TextArrowCtaProps = {
  label?: string;
  className?: string;
};

export default function TextArrowCta({ label = "Read more", className }: TextArrowCtaProps) {
  return (
    <div className={clsx("w-full h-fit flex flex-row items-center justify-start gap-[6px]", className)}>
      <h4 className="ty-button text-[#ff5e2b]">{label}</h4>
      <svg
        width="12"
        height="12"
        viewBox="0 0 12 12"
        fill="none"
        xmlns="http://www.w3.org/2000/svg"
        className="size-[12px]"
      >
        <path
          d="M0.697525 -4.94047e-07L10.9039 -4.79125e-08C11.5093 2.12227e-07 12 0.490745 12 1.09611V11.3025C12 11.6877 11.6877 12 11.3025 12C10.9172 12 10.6049 11.6877 10.6049 11.3025V2.38159L1.34951 11.6368C1.07711 11.9092 0.635565 11.9092 0.363165 11.6368C0.0907644 11.3644 0.0907644 10.9229 0.363165 10.6505L9.61861 1.39505L0.697525 1.39505C0.312293 1.39505 0 1.08276 0 0.697525C0 0.312292 0.312293 -5.10886e-07 0.697525 -4.94047e-07Z"
          fill="#FF5E2B"
        />
      </svg>
    </div>
  );
}
